#pragma once

#include <string>
#include <fstream>
#include "table.hpp"

void traverse_tree(Table &table, uint64_t page_no);
void traverse_index_tree(Table &table, uint64_t page_no);
