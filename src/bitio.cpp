#include "bitio.hpp"

BitWriter::BitWriter(std::ofstream& out) : out(out), buffer(0), bit_count(0) {}

//create a buffer of 8 bits and then pack the 8 bits together accordingly
void BitWriter::writeBit(int bit) {
    if (bit) buffer |= (1 << (7 - bit_count));
    bit_count++;
    if (bit_count == 8) {
        out.put(buffer);
        buffer = 0;
        bit_count = 0;
    }
}
//manual flushing for remaining bits
void BitWriter::flush() {
    if (bit_count > 0) {
        out.put(buffer);
        buffer = 0;
        bit_count = 0;
    }
}

BitReader::BitReader(std::ifstream& in) : in(in), buffer(0), bit_count(0) {}

//read bits from the buffer where it is stored in batches of 8
int BitReader::readBit() {
    //we have read all the 8 bits so now load the next batch of 8 bits
    if (bit_count == 0) {
        if (!in.read((char*)&buffer, 1)) return -1;
        bit_count = 8;
    }
    int bit = (buffer >> (bit_count - 1)) & 1;
    bit_count--;
    return bit;
}