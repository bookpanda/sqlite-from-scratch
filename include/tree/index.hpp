#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include "table.hpp"

std::unordered_map<std::string, uint64_t> get_indexes(const std::vector<Table> &tables);