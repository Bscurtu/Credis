
#include "../includes/lib.hh"

std::string execute(const std::vector<std::string>& cmd, Storage<std::string>& store)
{
    if (cmd.empty())
        return resp::error("ERR empty command");

    std::string name = cmd[0];
    for (char& c : name)
        c = std::toupper(static_cast<unsigned char>(c));
    size_t size = cmd.size();

    if (name == "PING")
        return resp::simple("PONG");

    if (name == "ECHO")
    {
        if (size != 2)
            return resp::error("ERR wrong number of arguments for 'echo' command");
        return resp::bulk(cmd[1]);
    }

    if (name == "SET")
    {
        if (size != 3)
            return resp::error("ERR wrong number of arguments for 'set' command");
        store.set(cmd[1], cmd[2]);
        return resp::simple("OK");
    }

    if (name == "GET")
    {
        if (size != 2)
            return resp::error("ERR wrong number of arguments for 'get' command");
        auto value = store.get(cmd[1]);
        if (!value)
            return resp::null_bulk();
        return resp::bulk(*value);
    }

    if (name == "DEL")
    {
        if (size != 2)
            return resp::error("ERR wrong number of arguments for 'del' command");
        return resp::integer(store.del(cmd[1]) ? 1 : 0);
    }

    return resp::error("ERR unknown command '" + cmd[0] + "'");
}