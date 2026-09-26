
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
#include <vector>
#include "data.hh"
#include "resp.hh"

namespace resp {
    std::string simple(const std::string& s);
    std::string error(const std::string& s);
    std::string integer(long long n);
    std::string bulk(const std::string& s);
    std::string null_bulk();
    std::optional<std::vector<std::string>> parse(std::string& buffer);
}