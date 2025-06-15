#include "tree/tree.hpp"
#include "globals.hpp"
#include "utils/utils.hpp"
#include "tree/page.hpp"

void traverse_tree(Table &table)
{
    uint32_t file_offset = (table.rootpage - 1) * page_size;
    database_file.seekg(file_offset);
    uint8_t page_header_size = check_page_header_size(database_file);

    database_file.seekg(file_offset + 3);
    uint16_t cell_count = check_bytes(database_file, 2);

    if (page_header_size == 8)
    {
        traverse_leaf_page(table, file_offset, cell_count);
    }
}