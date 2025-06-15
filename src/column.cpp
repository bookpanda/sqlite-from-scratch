#include "column.hpp"
#include <regex>
#include <iostream>
#include "utils/utils.hpp"

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
        // ([^,]+) = captures everything until a comma
        std::regex col_def(R"(\s*([a-zA-Z_][\w]*)\s+([^,\n]+))");

        auto begin = std::sregex_iterator(cols_raw.begin(), cols_raw.end(), col_def);
        auto end = std::sregex_iterator();

        const std::string whitespace = " \t\n\r";
        for (std::sregex_iterator i = begin; i != end; ++i)
        {
            std::string col_name = (*i)[1].str();
            std::string col_type = split_by_delim((*i)[2].str(), " ")[0]; // integer primary key autoincrement
            columns.push_back({col_name, col_type});
        }
    }

    return columns;
}