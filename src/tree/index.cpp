#include "tree/index.hpp"

std::unordered_map<std::string, uint64_t> get_indexes(const std::vector<Table> &tables)
{
    std::unordered_map<std::string, uint64_t> indexes;
    for (auto &table : tables)
    {
        if (table.type != "index")
            continue;

        indexes[table.name] = table.rootpage;
    }
    return indexes;
}