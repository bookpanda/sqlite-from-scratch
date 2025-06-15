#include "tree/index.hpp"
#include "utils/utils.hpp"
#include "globals.hpp"
#include "tree/page.hpp"

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

void traverse_index_leaf_page(Table &table, uint32_t file_offset, uint16_t cell_count, std::string where_val)
{
    for (uint16_t i = 0; i < cell_count; ++i)
    {
        database_file.seekg(file_offset + LEAF_PAGE_HEADER_SIZE + i * 2);
        uint16_t cell_offset = check_bytes(database_file, 2);

        database_file.seekg(file_offset + cell_offset);
        uint64_t record_size = read_varint(database_file);
        uint64_t header_size = read_varint(database_file);

        // assume only one column is indexed
        uint64_t key_size = read_serial_type_size(database_file);
        uint64_t rowid_size = read_serial_type_size(database_file);
        std::string key = read_string(database_file, key_size);
        uint64_t rowid = read_varint(database_file);
        if (key == where_val)
        {
            table.rows.push_back({{"rowid", rowid}});
        }

        // std::cout << "Cell " << i << ": " << std::endl;
        // std::cout << "  Key: " << key << std::endl;
        // std::cout << "  Row ID: " << rowid << std::endl;
    }
}

std::vector<IndexChildPage> traverse_index_interior_page(Table &table, uint32_t file_offset, uint16_t cell_count, std::string where_val)
{
    std::vector<IndexChildPage> child_pages;
    for (uint16_t i = 0; i < cell_count; ++i)
    {
        database_file.seekg(file_offset + INTERIOR_PAGE_HEADER_SIZE + i * 2);
        uint16_t cell_offset = check_bytes(database_file, 2);

        database_file.seekg(file_offset + cell_offset);
        uint32_t left_child_page = check_bytes(database_file, 4);
        uint64_t key_payload_size = read_varint(database_file);
        uint64_t header_size = read_varint(database_file);

        // assume only one column is indexed
        uint64_t key_size = read_serial_type_size(database_file);
        uint64_t rowid_size = read_serial_type_size(database_file);
        std::string key = read_string(database_file, key_size);
        uint64_t rowid = read_varint(database_file);
        if (key == where_val)
        {
            table.rows.push_back({{"rowid", rowid}});
        }

        // std::cout << "Cell " << i << ": " << std::endl;
        // std::cout << "  Left Child Page: " << left_child_page << std::endl;
        // std::cout << "  Key: " << key << std::endl;
        // std::cout << "  Row ID: " << rowid << std::endl;

        child_pages.push_back({left_child_page, key, rowid});
        if (i == cell_count - 1)
        {
            database_file.seekg(file_offset + LEAF_PAGE_HEADER_SIZE);
            uint32_t right_child_page = check_bytes(database_file, 4);
            child_pages.push_back({right_child_page, key, 0});
        }
    }

    return child_pages;
}