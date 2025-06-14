#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include "utils/utils.hpp"

int main(int argc, char *argv[])
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cerr << "Logs from your program will appear here" << std::endl;

    if (argc != 3)
    {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string database_file_path = argv[1];
    std::string command = argv[2];

    std::ifstream database_file(database_file_path, std::ios::binary);
    if (!database_file)
    {
        std::cerr << "Failed to open the database file" << std::endl;
        return 1;
    }

    if (command == ".dbinfo")
    {
        // seekg() overrides the current position in the file
        // read() moves the current position forward by the number of bytes read

        // skip the first 16 bytes of the header, it is "magic header"
        // "SQLite format 3\0"
        database_file.seekg(16);
        // page size is stored in the next 2 bytes
        char buffer[2];
        database_file.read(buffer, 2);
        unsigned short page_size = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));
        std::cout << "database page size: " << page_size << std::endl;

        database_file.seekg(103);
        char buffer2[2];
        database_file.read(buffer2, 2);
        unsigned int page_count = ((static_cast<unsigned char>(buffer2[1])) | (static_cast<unsigned char>(buffer2[0]) << 8));
        std::cout << "number of tables: " << page_count << std::endl;
    }
    else if (command == ".tables")
    {
        int offset = 100; // db header offset
        database_file.seekg(offset);
        uint8_t header_size = check_page_header_size(database_file);

        database_file.seekg(offset + 3);
        uint16_t cell_count = check_2_bytes(database_file);

        offset += header_size;
        std::vector<std::string> table_names;
        for (uint16_t i = 0; i < cell_count; ++i)
        {
            database_file.seekg(offset + i * 2);
            uint16_t cell_offset = check_2_bytes(database_file);

            database_file.seekg(cell_offset);
            uint64_t record_size = read_varint(database_file);
            uint64_t rowid = read_varint(database_file);
            uint64_t header_size = read_varint(database_file);

            uint64_t schema_type_size = read_serial_type_size(database_file);
            uint64_t schema_name_size = read_serial_type_size(database_file);
            uint64_t schema_tbl_name_size = read_serial_type_size(database_file);
            uint64_t schema_rootpage_size = read_serial_type_size(database_file);
            uint64_t schema_sql_size = read_serial_type_size(database_file);

            // std::cout << "Cell " << i << ": " << std::endl;
            // std::cout << "  Row ID: " << rowid << std::endl;
            // std::cout << "  Record Size: " << record_size << std::endl;
            // std::cout << "  Record Header Size: " << header_size << std::endl;
            // std::cout << "  Schema Type Size: " << schema_type_size << std::endl;
            // std::cout << "  Schema Name Size: " << schema_name_size << std::endl;
            // std::cout << "  Schema Table Size: " << schema_tbl_name_size << std::endl;
            // std::cout << "  Schema Root Page Size: " << schema_rootpage_size << std::endl;
            // std::cout << "  Schema SQL Size: " << schema_sql_size << std::endl;

            std::streampos record_body_start = database_file.tellg();
            std::streamoff offset = static_cast<std::streamoff>(record_body_start);
            database_file.seekg(offset + schema_type_size + schema_name_size);

            char buffer[schema_tbl_name_size];
            database_file.read(buffer, schema_tbl_name_size);
            std::string table_name(buffer, schema_tbl_name_size);
            if (table_name != "sqlite_sequence" && !table_name.empty())
            {
                table_names.push_back(table_name);
            }
            // std::cout << "  Table Name: " << table_name << std::endl;
        }

        for (const auto &name : table_names)
        {
            std::cout << name << " ";
        }
    }

    return 0;
}
