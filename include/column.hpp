#pragma once

#include <string>
#include <vector>

struct Column
{
    std::string name;
    std::string type;
};

std::vector<Column> parse_create_table(const std::string &sql);