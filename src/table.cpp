#include "table.hpp"
#include "utils/utils.hpp"

Table::Table(const std::string &type,
             const std::string &name,
             const std::string &tbl_name,
             const std::string &rootpage,
             const std::string &sql)
{
    this->type = type;
    this->name = name;
    this->tbl_name = tbl_name;
    this->rootpage = rootpage;
    this->sql = sql;
}

void Table::print() const
{
    std::cout << "Table: " << name << std::endl;
    std::cout << "  Table Name: " << tbl_name << std::endl;
    std::cout << "  Type: " << type << std::endl;
    std::cout << "  Root Page: " << rootpage << std::endl;
    std::cout << "  SQL: " << std::endl
              << sql << std::endl
              << std::endl;
}

std::vector<Table> get_tables(std::ifstream &database_file)
{
    int offset = 100; // db header offset
    database_file.seekg(offset);
    uint8_t header_size = check_page_header_size(database_file);

    database_file.seekg(offset + 3);
    uint16_t cell_count = check_2_bytes(database_file);

    offset += header_size;
    std::vector<Table> tables;

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

        std::cout << "Cell " << i << ": " << std::endl;
        std::cout << "  Row ID: " << rowid << std::endl;
        std::cout << "  Record Size: " << record_size << std::endl;
        std::cout << "  Record Header Size: " << header_size << std::endl;
        std::cout << "  Schema Type Size: " << schema_type_size << std::endl;
        std::cout << "  Schema Name Size: " << schema_name_size << std::endl;
        std::cout << "  Schema Table Size: " << schema_tbl_name_size << std::endl;
        std::cout << "  Schema Root Page Size: " << schema_rootpage_size << std::endl;
        std::cout << "  Schema SQL Size: " << schema_sql_size << std::endl;

        std::streampos record_body_start = database_file.tellg();
        std::streamoff offset = static_cast<std::streamoff>(record_body_start);
        database_file.seekg(offset);

        char type_buffer[schema_type_size];
        database_file.read(type_buffer, schema_type_size);
        std::string type(type_buffer, schema_type_size);

        char name_buffer[schema_name_size];
        database_file.read(name_buffer, schema_name_size);
        std::string name(name_buffer, schema_name_size);

        char tbl_name_buffer[schema_tbl_name_size];
        database_file.read(tbl_name_buffer, schema_tbl_name_size);
        std::string tbl_name(tbl_name_buffer, schema_tbl_name_size);

        char rootpage_buffer[schema_rootpage_size];
        database_file.read(rootpage_buffer, schema_rootpage_size);
        std::string rootpage(rootpage_buffer, schema_rootpage_size);

        char sql_buffer[schema_sql_size];
        database_file.read(sql_buffer, schema_sql_size);
        std::string sql(sql_buffer, schema_sql_size);

        tables.push_back(Table(type, name, tbl_name, rootpage, sql));
    }

    for (const auto &table : tables)
    {
        table.print();
    }

    return tables;
}