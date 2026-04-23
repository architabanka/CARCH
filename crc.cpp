#include <iostream>
#include <cstdint>
#include <vector>

// Generator polynomial (in reversed hexadecimal representation)
// = x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x^1 + x^0.
uint32_t polynomial = 0xEDB88320;

static std::vector<uint32_t> lookup_table()
{
    std::vector<uint32_t> table(256);

    for(int i=0; i<256; i++)
    {
        table[i] = i; // A byte with value = i

        for(int j=0; j<8; j++)
        {
            if (table[i] & 1)   // If lowest bit is 1, then shift right and xor with polynomial
                table[i] = (table[i] >> 1) ^ polynomial;
            else                // Shift right by 1
                table[i] >>= 1;
        }
    }

    return table;
}

std::vector<uint32_t> crc_table = lookup_table();

uint32_t generate_checksum(std::vector<uint8_t> data)
{
    uint32_t crc32 = 0xFFFFFFFF;
    for(uint8_t byte: data) // Process data
        crc32 ^= crc_table[byte];
    crc32 ^= 0xFFFFFFFF; // Take complement

    return crc32;
}