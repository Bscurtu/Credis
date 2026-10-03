
#include "../includes/lib.hh"

namespace resp {

    namespace {
        const std::string separator = "\r\n";
    }

    std::string simple(const std::string& s)
    {
        return "+" + s + separator;
    }

    std::string error(const std::string& s)
    {
        return "-" + s + separator;
    }

    std::string integer(long long n)
    {
        return ":" + std::to_string(n) + separator;
    }

    std::string bulk(const std::string& s)
    {
        return "$" + std::to_string(s.size()) + separator + s + separator;
    }

    std::string null_bulk()
    {
        return "$-1" + separator;
    }

    std::string array(const std::vector<std::string>& items)
    {
        std::string out = "*" + std::to_string(items.size()) + separator;
        for (const auto& item : items)
            out += bulk(item);
        return out;
    }

    std::optional<std::vector<std::string>> parse(std::string& buffer);
};

namespace parser {

    static int read_number(const std::string& s, size_t& i)
    {
        int n = 0;
        while (i < s.size() && s[i] >= '0' && s[i] <= '9')
        {
            n = n * 10 + (s[i] - '0');
            i++;
        }
        i += 2;
        return n;
    }

    static std::optional<int> read_header(const std::string& buffer, size_t& i, char prefix)
    {
        if (i >= buffer.size() || buffer[i] != prefix)
            return std::nullopt;
        if (buffer.find("\r\n", i) == std::string::npos)
            return std::nullopt;
        i++;
        return read_number(buffer, i);
    }

    std::optional<std::vector<std::string>> parse_command(std::string& buffer)
    {
        std::vector<std::string> parsed;
        size_t i = 0;

        auto n = read_header(buffer, i, '*');
        if (!n)
            return std::nullopt;

        for (int k = 0; k < *n; k++)
        {
            auto size = read_header(buffer, i, '$');
            if (!size)
                return std::nullopt;
            if (i + *size + 2 > buffer.size())
                return std::nullopt;
            parsed.push_back(buffer.substr(i, *size));
            i += *size + 2;
        }

        buffer.erase(0, i);
        return parsed;
    }
}