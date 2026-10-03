#include "../includes/lib.hh"
#include <cctype>
#include <filesystem>
#include <iterator>

Aof::Aof(const std::string& file_path): path(file_path), out(file_path, std::ios::app | std::ios::binary){}

bool Aof::is_open() const
{
    return out.is_open();
}

bool Aof::is_write_command(const std::vector<std::string>& cmd)
{
    if (cmd.empty())
        return false;
    std::string command = cmd[0];
    for (char& c : command)
        c = std::toupper(static_cast<unsigned char>(c));
    if (command == "SET" || command == "DEL" || command == "EXPIRE" || command == "PEXPIRE" || command == "PEXPIREAT")
        return true;
    else
        return false;
}

size_t Aof::load(Storage& store)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return 0;

    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const size_t total_size = content.size();
    size_t loaded = 0;
    while (auto cmd = parser::parse_command(content))
    {
        execute(*cmd, store);
        loaded++;
    }
    if (!content.empty())
    {
        size_t valid_size = total_size - content.size();
        std::filesystem::resize_file(path, valid_size);
    }
    return loaded;
}

std::vector<std::string> Aof::to_absolute(const std::vector<std::string>& cmd)
{
    std::string name = cmd[0];
    for (char& c : name)
        c = std::toupper(static_cast<unsigned char>(c));

    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    if (name == "EXPIRE") {
        long long secs = std::stoll(cmd[2]);
        return {"PEXPIREAT", cmd[1], std::to_string(now_ms + secs * 1000)};
    }
    if (name == "PEXPIRE") {
        long long ms = std::stoll(cmd[2]);
        return {"PEXPIREAT", cmd[1], std::to_string(now_ms + ms)};
    }
    return cmd;
}

void Aof::append(const std::vector<std::string>& cmd)
{
    if (!is_write_command(cmd))
        return;

    std::string line = resp::array(to_absolute(cmd));

    std::lock_guard<std::mutex> lock(mtx);
    out.write(line.data(), line.size());
    out.flush();
}