#include "tree/index.hpp"
#include "utils/utils.hpp"

std::unordered_map<std::string, uint64_t> get_indexes(const std::vector<Table> &tables)
{
    std::unordered_map<std::string, uint64_t> indexes;
    for (auto &table : tables)
    {
        if (table.type != "index")
            continue;

        auto parts = split_by_delim(table.name, "_");
        indexes[parts.back()] = table.rootpage;
    }
    return indexes;
}