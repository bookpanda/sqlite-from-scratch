#include "utils/utils.hpp"

uint64_t read_varint(std::ifstream &file)
{
    uint64_t value = 0;
    for (int i = 0; i < 9; ++i)
    {
        char byte;
        file.read(&byte, 1);
        // make sure it is unsigned
        unsigned char ubyte = static_cast<unsigned char>(byte);

        // append lower 7 bits
        if (i < 8)
        {
            value = (value << 7) | (ubyte & 0x7F);
            if (!(ubyte & 0x80))
                break; // MSB is 0 -> last byte
        }
        else
        {
            // 9th byte is all 8 bits
            value = (value << 8) | ubyte;
        }
    }
    return value;
}