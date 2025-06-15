#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include "utils/utils.hpp"
#include "table.hpp"
#include "globals.hpp"
#include "query.hpp"
#include "tree/index.hpp"
#include <algorithm>

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
                // std::cout << table.name << ", " << table.type << std::endl; // print table name
            }
        }
    }
    else
    {
        auto indexes = get_indexes(tables);
        auto query = parse_sql(command);
        auto selectedTable = query.table;
        // std::cout << "Selected table: " << selectedTable << std::endl;
        // std::cout << "Columns: ";
        // for (const auto &col : query.columns)
        // {
        //     std::cout << col << ", ";
        // }
        // std::cout << std::endl;
        // std::cout << "Where: " << query.where_col << " = " << query.where_val << std::endl;
        // std::cout << "Indexes: " << std::endl;
        // for (const auto &index : indexes)
        // {
        //     std::cout << "  " << index.first << ": " << index.second << std::endl;
        // }

        for (auto &table : tables)
        {
            if (table.tbl_name != selectedTable)
                continue;

            // table.print();
            if (indexes.find(query.where_col) != indexes.end())
            {
                // fetch using index, populate table's rows
                table.fetch_data_with_index(indexes[query.where_col], query.where_val);
                // std::cout << "Rows fetched using index: " << table.rows.size() << std::endl;
                std::vector<std::uint64_t> row_ids;
                for (const auto &row : table.rows)
                {
                    auto cell = row.at("rowid");
                    if (std::holds_alternative<uint64_t>(cell))
                    {
                        // std::cout << std::get<uint64_t>(cell) << std::endl;
                        row_ids.push_back(std::get<uint64_t>(cell));
                    }
                }
                sort(row_ids.begin(), row_ids.end());
                for (const auto &row_id : row_ids)
                {
                    // std::cout << "Fetching data for row ID: " << row_id << std::endl;
                    table.fetch_data(row_id);
                }
                for (const auto &row : table.rows)
                {
                    if (row.find("id") != row.end() && std::holds_alternative<uint64_t>(row.at("id")))
                    {
                        std::cout << std::get<uint64_t>(row.at("id")) << "|" << std::get<std::string>(row.at("name")) << std::endl;
                    }
                }
                break;
            }

            table.fetch_data();
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

    return 0;
}
