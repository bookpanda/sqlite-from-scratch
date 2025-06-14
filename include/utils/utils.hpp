#pragma once

#include <iostream>
#include <fstream>
#include <cstdint>

uint64_t read_varint(std::ifstream &file);
uint8_t check_page_header_size(std::ifstream &file);
uint16_t check_2_bytes(std::ifstream &file);
uint64_t read_serial_type_size(std::ifstream &file);