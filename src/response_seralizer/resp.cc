
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

    std::vector<std::string> parse_command(const std::string& received)
    {
        std::vector<std::string> parsed;
        size_t i = 0;

        if (received[i] != '*')
            return parsed;
        i++;
        int n = read_number(received, i);
        for (int k = 0; k < n; k++)
        {
            if (received[i] != '$')
                return {};
            i++;
            int size = read_number(received, i);
            parsed.push_back(received.substr(i, size));
            i += size + 2;
        }
        return parsed;
    }
};