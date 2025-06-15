#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include "table.hpp"
#include "page.hpp"

struct IndexChildPage
{
    uint32_t left_child_page;
    std::string key;
    uint64_t rowid;
};

std::unordered_map<std::string, uint64_t> get_indexes(const std::vector<Table> &tables);
void traverse_index_leaf_page(Table &table, uint32_t file_offset, uint16_t cell_count, std::string where_val);
std::vector<IndexChildPage> traverse_index_interior_page(Table &table, uint32_t file_offset, uint16_t cell_count, std::string where_val);