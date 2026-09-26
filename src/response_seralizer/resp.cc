
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
}