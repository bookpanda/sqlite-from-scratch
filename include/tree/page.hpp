#pragma once

#include <string>
#include <fstream>
#include "table.hpp"

struct ChildPage
{
    uint32_t left_child_page;
    uint64_t rowid;
};

const uint8_t LEAF_PAGE_HEADER_SIZE = 8;
const uint8_t INTERIOR_PAGE_HEADER_SIZE = 12;

void traverse_leaf_page(Table &table, uint32_t file_offset, uint16_t cell_count, uint64_t rowid = -1);
std::vector<ChildPage> traverse_interior_page(Table &table, uint32_t file_offset, uint16_t cell_count);