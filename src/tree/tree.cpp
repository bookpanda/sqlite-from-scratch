#include "tree/tree.hpp"
#include "globals.hpp"
#include "utils/utils.hpp"

void traverse_tree(Table &table, std::ifstream &file)
{
    uint32_t file_offset = (table.rootpage - 1) * page_size;
    database_file.seekg(file_offset);
    uint8_t page_header_size = check_page_header_size(database_file);

    for (uint16_t i = 0; i < table.size(); ++i)
    {
        database_file.seekg(file_offset + page_header_size + i * 2);
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

        for (size_t j = 0; j < table.columns.size(); ++j)
        {
            const auto &column = table.columns[j];
            uint64_t data_size = column_sizes[j];
            // std::cout << "  Column: " << column.name << ", Type: " << column.type << ", Size: " << data_size << std::endl;

            if (column.type == "integer")
            {
                int64_t value = check_bytes(database_file, data_size);
                table.rows[i][column.name] = value;
            }
            else if (column.type == "text")
            {
                std::string value = read_string(database_file, data_size);
                table.rows[i][column.name] = value;
            }
            else if (column.type == "real")
            {
                double value = *reinterpret_cast<double *>(new char[data_size]);
                database_file.read(reinterpret_cast<char *>(&value), data_size);
                table.rows[i][column.name] = value;
            }
            else
            {
                table.rows[i][column.name] = nullptr; // placeholder for unsupported types
            }
        }
    }
}