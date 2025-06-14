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

uint8_t check_page_header_size(std::ifstream &file)
{
    uint8_t bytes[1];
    file.read(reinterpret_cast<char *>(bytes), 1);

    switch (bytes[0])
    {
    case 0x02:
        return 12;
    case 0x05:
        return 12;
    case 0x0a:
        return 8;
    case 0x0d:
        return 8;
    default:
        std::cerr << "Invalid page header size: " << bytes[0] << std::endl;
        break;
    }
}

uint64_t check_bytes(std::ifstream &file, uint64_t size)
{
    uint8_t bytes[size];
    file.read(reinterpret_cast<char *>(bytes), size);
    uint64_t value = 0;
    for (uint64_t i = 0; i < size; ++i)
    {
        value = (value << 8) | bytes[i];
    }
    return value;
}

uint64_t read_serial_type_size(std::ifstream &file)
{
    uint64_t serial_type = read_varint(file);
    switch (serial_type)
    {
    case 0:
        return 0; // NULL
    case 1:
        return 1; // 8-bit twos-complement integer
    case 2:
        return 2; // big-endian 16-bit twos-complement integer
    case 3:
        return 3; // big-endian 24-bit twos-complement integer
    case 4:
        return 4; // big-endian 32-bit twos-complement integer
    case 5:
        return 6; // big-endian 48-bit twos-complement integer
    case 6:
        return 8; // big-endian 64-bit twos-complement integer
    case 7:
        return 8; // big-endian IEEE 754-2008 64-bit floating point number
    case 8:
        return 0; // integer 0
    case 9:
        return 0; // integer 1
    case 10:
        return 0; // reserved
    case 11:
        return 0; // reserved
    default:
        if (serial_type >= 12 && serial_type % 2 == 0)
        {
            return (serial_type - 12) / 2; // BLOB that is (N-12)/2 bytes in length
        }
        else if (serial_type >= 13 && serial_type % 2 == 1)
        {
            return (serial_type - 13) / 2; // string in the text encoding and (N-13)/2 bytes in length. The nul terminator is not stored
        }
        else
        {
            std::cerr << "Unknown serial type: " << serial_type << std::endl;
            return 0;
        }
    }
}

std::vector<std::string> split_by_delim(const std::string &s, const std::string &delim)
{
    std::vector<std::string> result;
    size_t pos = 0, prev = 0;

    while ((pos = s.find(delim, prev)) != std::string::npos)
    {
        result.push_back(s.substr(prev, pos - prev));
        prev = pos + delim.size();
    }
    result.push_back(s.substr(prev));

    return result;
}