
template <typename K>
class Storage
{
    private:
        using Clock = std::chrono::steady_clock;
        std::mutex mtx;

        struct Entry {
            std::string value;
            std::optional<Clock::time_point> expires_at;
        };

        std::unordered_map<K, Entry> data;

        static bool is_expired(const Entry& e)
        {
            return e.expires_at && Clock::now() >= *e.expires_at;
        }

    public:
        void set(const K& key, std::string value)
        {
            data.insert_or_assign(key, Entry{std::move(value), std::nullopt});
        }

        std::optional<std::string> get(const K& key)
        {
            auto it = data.find(key);
            if (it == data.end())
                return std::nullopt;
            if (is_expired(it->second)) {
                data.erase(it);
                return std::nullopt;
            }
            return it->second.value;
        }

        bool del(const K& key)
        {
            return data.erase(key) > 0;
        }

        bool expire(const K& key, std::chrono::seconds ttl)
        {
            auto it = data.find(key);
            if (it == data.end() || is_expired(it->second))
                return false;
            it->second.expires_at = Clock::now() + ttl;
            return true;
        }

        long long ttl(const K& key)
        {
            std::lock_guard<std::mutex> lock(mtx);

            auto it = data.find(key);
            if (it == data.end())
                return -2;
            if (is_expired(it->second)) {
                data.erase(it);
                return -2;
            }
            if (!it->second.expires_at)
                return -1;
            auto remaining = *it->second.expires_at - Clock::now();
            return std::chrono::round<std::chrono::seconds>(remaining).count();
        }

        size_t purge_expired()
        {
            std::lock_guard<std::mutex> lock(mtx);
            size_t removed = 0;
            for (auto it = data.begin(); it != data.end(); )
            {
                if (is_expired(it->second)) {
                    it = data.erase(it);
                    removed++;
                } else {
                    it++;
                }
            }
            return removed;
        }
};