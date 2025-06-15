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

void traverse_index_leaf_page(Table &table, uint32_t file_offset, uint16_t cell_count)
{
}

std::vector<ChildPage> traverse_index_interior_page(Table &table, uint32_t file_offset, uint16_t cell_count)
{
    std::vector<ChildPage> child_pages;
    for (uint16_t i = 0; i < cell_count; ++i)
    {
        database_file.seekg(file_offset + INTERIOR_PAGE_HEADER_SIZE + i * 2);
        uint16_t cell_offset = check_bytes(database_file, 2);

        database_file.seekg(file_offset + cell_offset);
        uint32_t left_child_page = check_bytes(database_file, 4);
        uint64_t key_payload_size = read_varint(database_file);
        uint64_t header_size = read_varint(database_file);

        std::vector<uint64_t> column_sizes;
        for (const auto &column : table.columns)
        {
            uint64_t data_size = read_serial_type_size(database_file);
            column_sizes.push_back(data_size);
        }

        Row row = {};
        for (size_t j = 0; j < table.columns.size(); ++j)
        {
            const auto &column = table.columns[j];
            uint64_t data_size = column_sizes[j];
            // std::cout << "  Column: " << column.name << ", Type: " << column.type << ", Size: " << data_size << std::endl;

            if (column.type == "integer")
            {
                int64_t value = check_bytes(database_file, data_size);
                row[column.name] = value;
                // row[column.name] = rowid;
            }
            else if (column.type == "text")
            {
                std::string value = read_string(database_file, data_size);
                row[column.name] = value;
            }
            else if (column.type == "real")
            {
                double value = *reinterpret_cast<double *>(new char[data_size]);
                database_file.read(reinterpret_cast<char *>(&value), data_size);
                row[column.name] = value;
            }
            else
            {
                row[column.name] = nullptr; // placeholder for unsupported types
            }
        }
        uint64_t rowid = read_varint(database_file);
        row["id"] = rowid;
        std::cout << "Cell " << i << ": " << std::endl;
        std::cout << "  Row ID: " << rowid << std::endl;
        std::cout << "  Left Child Page: " << left_child_page << std::endl;
        for (const auto &[col_name, cell] : row)
        {
            if (std::holds_alternative<int64_t>(cell))
                std::cout << "  " << col_name << ": " << std::get<int64_t>(cell) << std::endl;
            else if (std::holds_alternative<double>(cell))
                std::cout << "  " << col_name << ": " << std::get<double>(cell) << std::endl;
            else if (std::holds_alternative<std::string>(cell))
                std::cout << "  " << col_name << ": " << std::get<std::string>(cell) << std::endl;
            else
                std::cout << "  " << col_name << ": NULL" << std::endl;
        }
        child_pages.push_back({left_child_page, rowid});
    }

    database_file.seekg(file_offset + LEAF_PAGE_HEADER_SIZE);
    uint32_t rightmost_page = check_bytes(database_file, 4);
    child_pages.push_back({rightmost_page, 0});

    return child_pages;
}