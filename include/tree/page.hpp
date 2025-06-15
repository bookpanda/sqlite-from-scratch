#pragma once

#include <string>
#include <fstream>
#include "table.hpp"

struct ChildPage
{
    uint32_t left_child_page;
    uint64_t rowid;
};

void traverse_leaf_page(Table &table, uint32_t file_offset, uint16_t cell_count);
std::vector<ChildPage> traverse_interior_page(Table &table, uint32_t file_offset, uint16_t cell_count);