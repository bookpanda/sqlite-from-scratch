#pragma once

#include <string>
#include <fstream>
#include "table.hpp"

void traverse_tree(Table &table, uint64_t page_no, uint64_t rowid = -1);
void traverse_index_tree(Table &table, uint64_t page_no, std::string where_val);
