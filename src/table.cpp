#include "table.hpp"
#include "utils/utils.hpp"
#include "globals.hpp"

Table::Table(const std::string &type,
             const std::string &name,
             const std::string &tbl_name,
             const std::uint64_t &rootpage,
             const std::string &sql)
    : type(type), name(name), tbl_name(tbl_name), rootpage(rootpage), sql(sql)
{
    database_file.seekg((rootpage - 1) * page_size + 3); // offset 3 bytes from start of page header
    _size = check_bytes(database_file, 2);
    _columns = parse_create_table(sql);
    _rows.resize(_size);
}

int Table::size() const
{
    return _size;
}

void Table::fetch_data()
{
    if (_fetched)
    {
        std::cout << "Data already fetched for table: " << tbl_name << std::endl;
        return;
    }

    uint32_t offset = (rootpage - 1) * page_size;
    database_file.seekg(offset);
    uint8_t page_header_size = check_page_header_size(database_file);
    offset += page_header_size;

    for (uint16_t i = 0; i < _size; ++i)
    {
        database_file.seekg(offset + i * 2);
        uint16_t cell_offset = check_bytes(database_file, 2);

        database_file.seekg(cell_offset);
        uint64_t record_size = read_varint(database_file);
        uint64_t rowid = read_varint(database_file);
        uint64_t header_size = read_varint(database_file);

        std::vector<uint64_t> column_sizes;
        for (const auto &column : _columns)
        {
            uint64_t data_size = read_serial_type_size(database_file);
            column_sizes.push_back(data_size);
            // std::string data = read_string(database_file, data_size);
            // _rows[i][column.name] = data; // Assuming _rows is a vector of Row (unordered_map)
        }

        for (size_t j = 0; j < _columns.size(); ++j)
        {
            const auto &column = _columns[j];
            uint64_t data_size = column_sizes[j];

            if (column.type == "int")
            {
                int64_t value = check_bytes(database_file, data_size);
                _rows[i][column.name] = value;
            }
            else if (column.type == "text")
            {
                std::string value = read_string(database_file, data_size);
                _rows[i][column.name] = value;
            }
            else if (column.type == "real")
            {
                double value = *reinterpret_cast<double *>(new char[data_size]);
                database_file.read(reinterpret_cast<char *>(&value), data_size);
                _rows[i][column.name] = value;
            }
            else
            {
                _rows[i][column.name] = nullptr; // placeholder for unsupported types
            }
        }
    }
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
              << sql << std::endl
              << std::endl;
}

static std::string read_string(std::ifstream &file, uint64_t size)
{
    std::string result(size, '\0');
    file.read(&result[0], size);
    return result;
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

    for (const auto &table : tables)
    {
        table.print();
    }

    return tables;
}