#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include "utils/utils.hpp"
#include "table.hpp"
#include "globals.hpp"

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

    // seekg() overrides the current position in the file
    // read() moves the current position forward by the number of bytes read

    // skip the first 16 bytes of the header, it is "magic header"
    // "SQLite format 3\0"
    database_file.seekg(16);
    // page size is stored in the next 2 bytes
    page_size = check_2_bytes(database_file);

    if (command == ".dbinfo")
    {
        std::cout << "database page size: " << page_size << std::endl;

        database_file.seekg(103);
        char buffer2[2];
        database_file.read(buffer2, 2);
        unsigned int page_count = ((static_cast<unsigned char>(buffer2[1])) | (static_cast<unsigned char>(buffer2[0]) << 8));
        std::cout << "number of tables: " << page_count << std::endl;
    }
    else if (command == ".tables")
    {
        auto tables = get_tables(database_file);
        for (const auto &table : tables)
        {
            if (table.tbl_name != "sqlite_sequence" && !table.tbl_name.empty())
            {
                std::cout << table.tbl_name << " ";
            }
        }
    }

    return 0;
}
