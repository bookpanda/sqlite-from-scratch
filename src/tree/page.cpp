#include "tree/page.hpp"
#include "globals.hpp"
#include "utils/utils.hpp"

void traverse_leaf_page(Table &table, uint32_t file_offset, uint16_t cell_count, uint64_t wanted_rowid)
{
    for (uint16_t i = 0; i < cell_count; ++i)
    {
        database_file.seekg(file_offset + LEAF_PAGE_HEADER_SIZE + i * 2);
        uint16_t cell_offset = check_bytes(database_file, 2);

        database_file.seekg(file_offset + cell_offset);
        uint64_t record_size = read_varint(database_file);
        uint64_t rowid = read_varint(database_file);
        uint64_t header_size = read_varint(database_file);
        // std::cout << "Row " << i << ": " << std::endl;
        // std::cout << "  Row ID: " << rowid << std::endl;
        // std::cout << "  Record Size: " << record_size << std::endl;
        // std::cout << "  Record Header Size: " << header_size << std::endl;

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
                // int64_t value = check_bytes(database_file, data_size);
                // row[column.name] = value;
                row[column.name] = rowid;
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
        auto _rowid = std::get<uint64_t>(row.at("id"));
        if (wanted_rowid == -1 || wanted_rowid == _rowid)
        {
            // std::cout << "rowid: " << rowid << ", _rowid: " << _rowid << std::endl;
            table.rows.push_back(row);
            if (wanted_rowid != -1 && wanted_rowid == _rowid)
                return;
        }
    }
}

std::vector<ChildPage> traverse_interior_page(Table &table, uint32_t file_offset, uint16_t cell_count)
{
    std::vector<ChildPage> child_pages;
    for (uint16_t i = 0; i < cell_count; ++i)
    {
        database_file.seekg(file_offset + INTERIOR_PAGE_HEADER_SIZE + i * 2);
        uint16_t cell_offset = check_bytes(database_file, 2);

        database_file.seekg(file_offset + cell_offset);
        uint32_t left_child_page = check_bytes(database_file, 4);
        uint64_t rowid = read_varint(database_file);
        // std::cout << "Cell " << i << ": " << std::endl;
        // std::cout << "  Row ID: " << rowid << std::endl;
        // std::cout << "  Left Child Page: " << left_child_page << std::endl;
        child_pages.push_back({left_child_page, rowid});
    }

    database_file.seekg(file_offset + LEAF_PAGE_HEADER_SIZE);
    uint32_t rightmost_page = check_bytes(database_file, 4);
    child_pages.push_back({rightmost_page, 0});

    return child_pages;
}