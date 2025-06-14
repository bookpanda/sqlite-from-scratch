#pragma once

#include <string>
#include <vector>

class Table
{
public:
    Table(
        const std::string &type,
        const std::string &name,
        const std::string &tbl_name,
        const std::string &rootpage,
        const std::string &sql);

    std::string type;
    std::string name;
    std::string tbl_name;
    std::string rootpage;
    std::string sql;

    void print() const;
};

std::vector<Table> get_tables(std::ifstream &file);