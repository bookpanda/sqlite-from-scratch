#pragma once

#include <string>
#include <vector>

struct ParsedQuery
{
    std::vector<std::string> columns;
    std::string table;
    std::string where_clause; // empty if none
};

ParsedQuery parse_sql(const std::string &query);