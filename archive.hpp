// WHY THIS FILE EXISTS:
//   This is the "controller" layer. It ties together:
//     - File I/O (reading/writing files from disk)
//     - Huffman compression/decompression
//     - The CARCH binary archive format
//
// ARCHIVE FILE FORMAT LAYOUT:
//   [CARCH]           5 bytes  - magic number to identify file type
//   [file_count]      4 bytes  - how many files are in this archive
//   --- repeated for each file ---
//   [filename\0]      variable - null-terminated filename string
//   [original_size]   8 bytes  - size of file before compression
//   [freq_table]    2048 bytes - 256 x uint64_t frequency counts
//   [compressed_size] 8 bytes  - size of compressed data in bytes
//   [compressed_data] variable - Huffman-encoded bits (tree + data)
#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "huffman.hpp"
#include "bitio.hpp"

namespace Archive {

    void createArchive(const std::string& archiveName, const std::vector<std::string>& fileNames);
    void extractArchive(const std::string& archiveName);

}
