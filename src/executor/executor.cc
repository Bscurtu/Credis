
#include "../includes/lib.hh"

std::string execute(const std::vector<std::string>& cmd, Storage<std::string>& store)
{
    if (cmd.empty())
        return resp::error("ERR empty command");

    std::string command = cmd[0];
    for (char& c : command)
        c = std::toupper(static_cast<unsigned char>(c));
    size_t size = cmd.size();

    if (command == "PING")
        return resp::simple("PONG");

    if (command == "ECHO")
    {
        if (size != 2)
            return resp::error("ERR wrong number of arguments for 'echo' command");
        return resp::bulk(cmd[1]);
    }

    if (command == "SET")
    {
        if (size != 3)
            return resp::error("ERR wrong number of arguments for 'set' command");
        store.set(cmd[1], cmd[2]);
        return resp::simple("OK");
    }

    if (command == "GET")
    {
        if (size != 2)
            return resp::error("ERR wrong number of arguments for 'get' command");
        auto value = store.get(cmd[1]);
        if (!value)
            return resp::null_bulk();
        return resp::bulk(*value);
    }

    if (command == "DEL")
    {
        if (size != 2)
            return resp::error("ERR wrong number of arguments for 'del' command");
        return resp::integer(store.del(cmd[1]) ? 1 : 0);
    }

    if (command == "TTL")
    {
        if (size != 2)
            return resp::error("ERR wrong number of arguments for 'ttl' command");
        return resp::integer(store.ttl(cmd[1]));
    }

    if (command == "EXPIRE")
    {
        if (size != 3)
            return resp::error("ERR wrong number of arguments for 'expire' command");
        long long secs;
        try {
            secs = std::stoll(cmd[2]);
        } catch (...) {
            return resp::error("ERR value is not an integer or out of range");
        }
        bool ok = store.expire(cmd[1], std::chrono::seconds(secs));
        return resp::integer(ok ? 1 : 0);
    }

    if (command == "PEXPIREAT")
    {
        if (size != 3)
            return resp::error("ERR wrong number of arguments for 'pexpireat' command");
        long long unix_ms;
        try {
            unix_ms = std::stoll(cmd[2]);
        } catch (...) {
            return resp::error("ERR value is not an integer or out of range");
        }
        return resp::integer(store.expire_at(cmd[1], unix_ms) ? 1 : 0);
    }

    if (command  == "PEXPIRE")
    {
        if (size != 3)
            return resp::error("ERR wrong number of arguments for 'pexpire' command");
        long long ms;
        try {
            ms = std::stoll(cmd[2]);
        } catch (...) {
            return resp::error("ERR value is not an integer or out of range");
        }
        return resp::integer(store.expire(cmd[1], std::chrono::milliseconds(ms)) ? 1 : 0);
    }

    return resp::error("ERR unknown command '" + cmd[0] + "'");
}