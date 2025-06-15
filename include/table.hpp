#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <unordered_map>
#include <variant>
#include "column.hpp"

using Cell = std::variant<int64_t, double, std::string, std::nullptr_t>;
using Row = std::unordered_map<std::string, Cell>;

class Table
{
public:
    Table(
        const std::string &type,
        const std::string &name,
        const std::string &tbl_name,
        const uint64_t &rootpage,
        const std::string &sql);

    std::string type;
    std::string name;
    std::string tbl_name;
    uint64_t rootpage;
    std::string sql;
    std::vector<Row> rows;
    std::vector<Column> columns;

    void print() const;
    size_t size() const;
    void fetch_data();

private:
    bool _fetched = false;
};

std::vector<Table> get_tables(std::ifstream &file);