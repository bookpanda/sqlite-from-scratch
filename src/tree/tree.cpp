#include "tree/tree.hpp"
#include "globals.hpp"
#include "utils/utils.hpp"
#include "tree/page.hpp"

void traverse_tree(Table &table, uint64_t page_no)
{
    uint32_t file_offset = (page_no - 1) * page_size;
    database_file.seekg(file_offset);
    uint8_t page_header_size = check_page_header_size(database_file);

    database_file.seekg(file_offset + 3);
    uint16_t cell_count = check_bytes(database_file, 2);

    if (page_header_size == 8)
    {
        traverse_leaf_page(table, file_offset, cell_count);
    }
    else if (page_header_size == 12)
    {
        auto child_pages = traverse_interior_page(table, file_offset, cell_count);
    }
    else
    {
        std::cerr << "Unknown page header size: " << static_cast<int>(page_header_size) << std::endl;
    }
}