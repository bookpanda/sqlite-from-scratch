#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

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

    void print() const;
    int size() const;

private:
    int _size;
};

std::vector<Table> get_tables(std::ifstream &file);