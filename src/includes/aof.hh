#pragma once
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

#include "data.hh"

class Aof
{
    private:
        std::string path;
        std::ofstream out;
        std::mutex mtx;

    public:
        explicit Aof(const std::string& file_path);

        bool is_open() const;
        size_t load(Storage<std::string>& store);
        void append(const std::vector<std::string>& cmd);
        static bool is_write_command(const std::vector<std::string>& cmd);
        
    private:
        static std::vector<std::string> to_absolute(const std::vector<std::string>& cmd);
};