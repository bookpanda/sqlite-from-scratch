#include "tree/tree.hpp"
#include "globals.hpp"
#include "utils/utils.hpp"
#include "tree/page.hpp"
#include "tree/index.hpp"

void traverse_tree(Table &table, uint64_t page_no, uint64_t rowid)
{
    uint32_t file_offset = (page_no - 1) * page_size;
    database_file.seekg(file_offset);
    uint8_t page_header_size = check_page_header_size(database_file);

    database_file.seekg(file_offset + 3);
    uint16_t cell_count = check_bytes(database_file, 2);

    // std::cout << "traversing page: " << page_no
    //           << ", header_size: " << static_cast<int>(page_header_size)
    //           << ", cell_count: " << cell_count << std::endl;

    if (page_header_size == 8)
    {
        traverse_leaf_page(table, file_offset, cell_count, rowid);
    }
    else if (page_header_size == 12)
    {
        auto child_pages = traverse_interior_page(table, file_offset, cell_count);
        if (rowid == -1)
        {

            for (const auto &child : child_pages)
            {
                // std::cout << "Traversing child page: " << child.left_child_page << std::endl;
                traverse_tree(table, child.left_child_page, rowid);
            }
            return;
        }

        for (const auto &child : child_pages)
        {
            if (child.rowid >= rowid)
            {
                // std::cout << "Key: " << child.rowid << ", traversing left child page: "
                //           << child.left_child_page << std::endl;
                traverse_tree(table, child.left_child_page, rowid);
                break;
            }
        }
        if (child_pages.back().rowid <= rowid)
        {
            traverse_tree(table, child_pages.back().left_child_page, rowid);
            return;
        }
    }
    else
    {
        std::cerr << "Unknown page header size: " << static_cast<int>(page_header_size) << std::endl;
    }
}

void traverse_index_tree(Table &table, uint64_t page_no, std::string where_val)
{
    // std::cout << "Traversing index tree for table: " << table.tbl_name << ", page: " << page_no << std::endl;
    uint32_t file_offset = (page_no - 1) * page_size;
    database_file.seekg(file_offset);
    uint8_t page_header_size = check_page_header_size(database_file);

    database_file.seekg(file_offset + 3);
    uint16_t cell_count = check_bytes(database_file, 2);

    if (page_header_size == 8)
    {
        traverse_index_leaf_page(table, file_offset, cell_count, where_val);
    }
    else if (page_header_size == 12)
    {
        auto child_pages = traverse_index_interior_page(table, file_offset, cell_count, where_val);
        if (child_pages[0].key >= where_val)
        {
            traverse_index_tree(table, child_pages[0].left_child_page, where_val);
            return;
        }
        if (child_pages.back().key <= where_val)
        {
            traverse_index_tree(table, child_pages.back().left_child_page, where_val);
            return;
        }
        for (const auto &child : child_pages)
        {
            if (child.key >= where_val)
            {
                // std::cout << "Key: " << child.key << ", traversing left child page: "
                //           << child.left_child_page << std::endl;
                traverse_index_tree(table, child.left_child_page, where_val);
            }
        }
        // auto last_child = child_pages.back();
        // if (last_child.key <= where_val)
        // {
        //     // std::cout << "Key: " << last_child.key << ", traversing right child page: "
        //     //           << last_child.left_child_page << std::endl;
        //     // this is rightmost child page
        //     traverse_index_tree(table, last_child.left_child_page, where_val);
        // }
    }
    else
    {
        std::cerr << "Unknown index page header size: " << static_cast<int>(page_header_size) << std::endl;
    }
}