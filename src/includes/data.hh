
template <typename K>
class Storage
{
    private:
        using Clock = std::chrono::steady_clock;

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
};