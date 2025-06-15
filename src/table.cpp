#include "table.hpp"
#include "utils/utils.hpp"
#include "globals.hpp"
#include "tree/tree.hpp"

Table::Table(const std::string &type,
             const std::string &name,
             const std::string &tbl_name,
             const std::uint64_t &rootpage,
             const std::string &sql)
    : type(type), name(name), tbl_name(tbl_name), rootpage(rootpage), sql(sql)
{
    columns = parse_create_table(sql);
    rows = std::vector<Row>(0); // empty vector
}

size_t Table::size() const
{
    return rows.size();
}

void Table::fetch_data()
{
    if (_fetched)
    {
        std::cout << "Data already fetched for table: " << tbl_name << std::endl;
        return;
    }
    traverse_tree(*this, rootpage);

    _fetched = true;
}

void Table::print() const
{
    std::cout << "Table: " << name << std::endl;
    std::cout << "  Table Name: " << tbl_name << std::endl;
    std::cout << "  Type: " << type << std::endl;
    std::cout << "  Root Page: " << rootpage << std::endl;
    std::cout << "  Size: " << size() << std::endl;
    std::cout << "  SQL: " << std::endl
              << sql << std::endl;
    std::cout << " Columns: " << std::endl;
    for (const auto &col : columns)
    {
        std::cout << "    " << col.name << " (" << col.type << ")" << std::endl;
    }

    if (_fetched)
    {
        std::cout << "  Rows: " << std::endl;
        for (const auto &row : rows)
        {
            for (const auto &col : columns)
            {
                if (row.find(col.name) != row.end())
                {
                    const Cell &cell = row.at(col.name);
                    if (std::holds_alternative<int64_t>(cell))
                        std::cout << col.name << ": " << std::get<int64_t>(cell) << ", ";
                    else if (std::holds_alternative<double>(cell))
                        std::cout << col.name << ": " << std::get<double>(cell) << ", ";
                    else if (std::holds_alternative<std::string>(cell))
                        std::cout << col.name << ": " << std::get<std::string>(cell) << ", ";
                    else
                        std::cout << col.name << ": NULL, ";
                }
            }
            std::cout << std::endl;
        }
    }
}

std::vector<Table> get_tables(std::ifstream &database_file)
{
    uint32_t offset = 100; // db header offset
    database_file.seekg(offset);
    uint8_t page_header_size = check_page_header_size(database_file);

    database_file.seekg(offset + 3);
    uint16_t cell_count = check_bytes(database_file, 2);

    offset += page_header_size;
    std::vector<Table> tables;

    for (uint16_t i = 0; i < cell_count; ++i)
    {
        database_file.seekg(offset + i * 2);
        uint16_t cell_offset = check_bytes(database_file, 2);

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

        std::string type = read_string(database_file, schema_type_size);
        std::string name = read_string(database_file, schema_name_size);
        std::string tbl_name = read_string(database_file, schema_tbl_name_size);
        uint64_t rootpage = check_bytes(database_file, schema_rootpage_size);
        std::string sql = read_string(database_file, schema_sql_size);

        tables.emplace_back(type, name, tbl_name, rootpage, sql);
    }

    return tables;
}