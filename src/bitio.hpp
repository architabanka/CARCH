// bitio.h  —  Bit-level I/O
// WHY THIS FILE EXISTS:
//   Normal fwrite() only works in whole bytes (8 bits at once).
//   Huffman codes are 1, 2, 3 ... N bits long — they don't
//   fit neatly into bytes. This file gives you two classes:
//   BitWriter → you give it bits one-by-one, it packs them
//               into bytes and writes to file
//   BitReader → reads bytes from file, gives you one bit at
//               a time for decoding

#pragma once
#include <iostream>
#include <fstream>
#include <cstdint>

class BitWriter {
    std::ofstream& out;
    uint8_t buffer;
    int bit_count;

public:
    BitWriter(std::ofstream& out);
    void writeBit(int bit);
    void flush();
};

class BitReader {
    std::ifstream& in;
    uint8_t buffer;
    int bit_count;

public:
    BitReader(std::ifstream& in);
    int readBit();
};
