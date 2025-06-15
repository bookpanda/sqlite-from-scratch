#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include "table.hpp"
#include "page.hpp"

std::unordered_map<std::string, uint64_t> get_indexes(const std::vector<Table> &tables);
void traverse_index_leaf_page(Table &table, uint32_t file_offset, uint16_t cell_count);
std::vector<ChildPage> traverse_index_interior_page(Table &table, uint32_t file_offset, uint16_t cell_count);