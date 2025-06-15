#pragma once

#include <string>
#include <vector>
#include "table.hpp"

struct ParsedQuery
{
    std::vector<std::string> columns;
    std::string table;
    std::string where_col; // empty if none
    std::string where_val; // empty if none
};

ParsedQuery parse_sql(const std::string &query);
void print_query_result(const Table &table, const ParsedQuery query);