#include "column.hpp"
#include <regex>
#include <iostream>

std::vector<Column> parse_create_table(const std::string &sql)
{
    std::vector<Column> columns;

    // outer () = capturing group to capture entire match
    // \( ... \) = match literal parentheses
    // ([\s\S]*) = inner capturing group: match any character (0 or more times)
    // \s = any whitespace character, \S = any non-whitespace character
    std::regex paren_contents(R"(\(([\s\S]*)\))");
    std::smatch match;
    if (std::regex_search(sql, match, paren_contents))
    {
        // CREATE TABLE t (id int, name text)
        // match[0]: the entire match e.g. (id int, name text)
        // match[1]: the first capturing group e.g. id int, name text
        std::string cols_raw = match[1];
        // \w = [A-Za-z0-9_]
        // (space) col_name type
        std::regex col_def(R"(\s*([a-zA-Z_][\w]*)\s+([a-zA-Z]+))");

        auto begin = std::sregex_iterator(cols_raw.begin(), cols_raw.end(), col_def);
        auto end = std::sregex_iterator();

        for (std::sregex_iterator i = begin; i != end; ++i)
        {
            // (*i)[1] = the first capturing group (column name)
            // (*i)[2] = the second capturing group (column type)
            columns.push_back({(*i)[1], (*i)[2]});
        }
    }

    return columns;
}