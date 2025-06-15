#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include "utils/utils.hpp"
#include "table.hpp"
#include "globals.hpp"
#include "query.hpp"

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

    database_file = std::ifstream(database_file_path, std::ios::binary);
    if (!database_file.is_open())
    {
        std::cerr << "Failed to open the database file: " << database_file_path << std::endl;
        return 1;
    }

    // seekg() overrides the current position in the file
    // read() moves the current position forward by the number of bytes read

    // skip the first 16 bytes of the header, it is "magic header"
    // "SQLite format 3\0"
    database_file.seekg(16);
    // page size is stored in the next 2 bytes
    page_size = check_bytes(database_file, 2);
    auto tables = get_tables(database_file);

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
        for (const auto &table : tables)
        {
            if (table.tbl_name != "sqlite_sequence" && !table.tbl_name.empty())
            {
                std::cout << table.tbl_name << " ";
                // std::cout << table.name << std::endl; // print table name
            }
        }
    }
    else
    {
        auto query = parse_sql(command);
        auto selectedTable = query.table;
        // std::cout << "Selected table: " << selectedTable << std::endl;
        // std::cout << "Columns: ";
        // for (const auto &col : query.columns)
        // {
        //     std::cout << col << ", ";
        // }
        // std::cout << std::endl;
        // std::cout << "Where clause: " << query.where_clause << std::endl;

        for (auto &table : tables)
        {
            if (table.tbl_name == selectedTable)
            {
                table.fetch_data();
                // table.print();
                if (query.columns.size() == 1)
                {
                    if (to_uppercase(query.columns[0]) == "COUNT(*)")
                        std::cout << table.size() << std::endl;
                    else
                        print_query_result(table, query);
                    break;
                }
                print_query_result(table, query);
                break;
            }
        }
    }

    return 0;
}
