
#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <iostream>
#include <cstring>
#include <chrono>
#include <cassert>
#include <iostream>
#include <thread>
#include <getopt.h>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>
#include <sys/socket.h>
#include <fstream>
#include <cctype>
#include <thread>
#include <iostream>
#include <vector>
#include "aof.hh"
#include "data.hh"


namespace resp {
    std::string simple(const std::string& s);
    std::string error(const std::string& s);
    std::string integer(long long n);
    std::string bulk(const std::string& s);
    std::string null_bulk();
    std::optional<std::vector<std::string>> parse(std::string& buffer);
    std::string array(const std::vector<std::string>& items);
}

namespace parser {
    std::optional<std::vector<std::string>> parse_command(std::string& buffer);
}

void handle_client(int client_fd, Storage& store, Aof& aof);

std::string execute(const std::vector<std::string>& cmd, Storage& store);

std::string array(const std::vector<std::string>& items);

void    tester();
int net_init();
void credis_run();