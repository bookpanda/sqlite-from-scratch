#pragma once

#include <string>
#include <fstream>
#include "table.hpp"

void traverse_leaf_page(Table &table, uint32_t file_offset);