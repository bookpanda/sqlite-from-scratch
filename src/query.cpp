#include <iostream>
#include <regex>
#include <sstream>
#include "query.hpp"
#include "table.hpp"

ParsedQuery parse_sql(const std::string &query)
{
    // Case-insensitive regex for SELECT ... FROM ... [WHERE ...]
    // . = any character except newline, ? after quantifier = lazy (non-greedy) match
    // (.+?) = get cols e.g. name, color
    //
    // (?: ...) = non-capturing group (WHERE is not in match[i]), ? = optional (0 or 1 time)
    // (?:\s+WHERE\s+(.+))?)
    std::regex sql_regex(
        R"(SELECT\s+(.+?)\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?)",
        std::regex_constants::icase);

    std::smatch match;
    ParsedQuery result;

    if (std::regex_search(query, match, sql_regex))
    {
        // match[1] = columns string
        // match[2] = table name
        // match[3] = where clause (optional)

        // split columns by comma
        std::string cols_str = match[1];
        std::stringstream ss(cols_str);
        std::string col;

        while (std::getline(ss, col, ','))
        {
            // trim whitespace
            size_t start = col.find_first_not_of(" \t");
            size_t end = col.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos)
                result.columns.push_back(col.substr(start, end - start + 1));
        }

        result.table = match[2];
        if (match.size() > 3 && match[3].matched)
        {
            std::string where_clause = match[3];
            std::string where_col, where_val;
            std::regex where_regex(R"((\w+)\s*=\s*'(.*?)')", std::regex_constants::icase);
            std::smatch where_match;

            if (std::regex_search(where_clause, where_match, where_regex))
            {
                result.where_col = where_match[1];
                result.where_val = where_match[2];
            }
            else
            {
                std::cerr << "Unsupported WHERE clause format: " << where_clause << std::endl;
            }
        }
    }
    return result;
}

void print_query_result(const Table &table, const ParsedQuery query)
{
    bool has_where = !query.where_col.empty();

    for (auto &row : table.rows)
    {
        // apply WHERE clause filter
        if (has_where)
        {
            auto it = row.find(query.where_col);
            if (it == row.end())
                continue;

            // reject if is NULL or does not match the value
            if (std::holds_alternative<std::nullptr_t>(it->second) ||
                std::get<std::string>(it->second) != query.where_val)
                continue;
        }

        std::vector<std::string> rowResult;
        for (auto &col : query.columns)
        {
            if (row.find(col) == row.end())
                continue;
            if (std::holds_alternative<std::nullptr_t>(row.at(col)))
                std::cout << "NULL" << std::endl;
            else if (std::holds_alternative<int64_t>(row.at(col)))
                rowResult.push_back(std::to_string(std::get<int64_t>(row.at(col))));
            else if (std::holds_alternative<uint64_t>(row.at(col)))
                rowResult.push_back(std::to_string(std::get<uint64_t>(row.at(col))));
            else if (std::holds_alternative<std::string>(row.at(col)))
                rowResult.push_back(std::get<std::string>(row.at(col)));
        }

        for (size_t i = 0; i < rowResult.size(); ++i)
        {
            std::cout << rowResult[i];
            if (i < rowResult.size() - 1)
                std::cout << "|";
        }
        std::cout << std::endl;
    }
}