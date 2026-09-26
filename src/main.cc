
#include "includes/lib.hh"

using namespace std::chrono_literals;
int main()
{

    std::optional<std::vector<std::string>> parse(std::string& buffer);



    // Existing value
    {
        Storage<std::string> s;
        s.set("a", "1");
        assert(s.get("a") == "1");
    }
    // The value of the key
    {
        Storage<std::string> s;
        assert(!s.get("nothing").has_value());
    }
    // Overwrite the data
    {
        Storage<std::string> s;
        s.set("a", "1");
        s.set("a", "2");
        assert(s.get("a") == "2");
    }
    // Remove the data
    {
        Storage<std::string> s;
        s.set("a", "1");
        assert(s.del("a") == true);
        assert(!s.get("a").has_value());
        assert(s.del("a") == false);
    }
    // Check if the data has expired
    {
        Storage<std::string> s;
        s.set("a", "1");
        s.expire("a", 1s);
        std::this_thread::sleep_for(1100ms);
        assert(!s.get("a").has_value());
    }
    // Set removes the expire time that has been set
    {
        Storage<std::string> s;
        s.set("a", "1");
        s.expire("a", 1s);
        s.set("a", "2");
        std::this_thread::sleep_for(1100ms);
        assert(s.get("a") == "2");
    }

    std::cout << "Every test about Storage has succeded\n";

    assert(resp::simple("OK") == "+OK\r\n");
    assert(resp::bulk("Hello") == "$4\r\nHello\r\n");
    assert(resp::integer(1) == ":1\r\n");
    assert(resp::null_bulk() == "$-1\r\n");
    return 0;
}