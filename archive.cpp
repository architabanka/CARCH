#include "archive.hpp"
#include "crc.hpp"
#include <fstream>
#include <cstring>

namespace Archive {

    void createArchive(const std::string& archiveName, const std::vector<std::string>& fileNames) {
        std::ofstream out(archiveName, std::ios::binary);
        if (!out) {
            std::cerr << "Error writing archive " << archiveName << std::endl;
            return;
        }

        out.write("CARCH", 5);
        uint32_t file_count = fileNames.size();
        out.write((char*)&file_count, 4);

        for (const std::string& fname : fileNames) {
            std::ifstream in(fname, std::ios::binary | std::ios::ate);
            if (!in) {
                std::cerr << "Warning: Cannot open file " << fname << ", skipping" << std::endl;
                continue;
            }
            
            uint64_t orig_size = in.tellg();
            in.seekg(0, std::ios::beg);

            std::vector<uint8_t> content(orig_size);
            if (orig_size > 0) in.read((char*)content.data(), orig_size);
            in.close();

            out.write(fname.c_str(), fname.length() + 1);
            out.write((char*)&orig_size, 8);

            // 32 Bit Cyclic Redundancy Chceksum
            uint32_t crc32 = generate_checksum(content);
            out.write((char*)&crc32, 4);

            uint64_t freq[256] = {0};
            //ADD : increment frequency for each character in ascii
            for(auto val:content) {
                freq[val]++;
            }

            std::priority_queue<Huffman::Node*, std::vector<Huffman::Node*>, Huffman::CompareNode> pq;

            // We cannot use uint8_t since it is always less than 255.
            // When incrementing further, it overflows back to zero.
            for (int i = 0; i < 256; i++) {
                // Avoid creating representations for unused characters
                if (freq[i] > 0) {
                    Huffman::Node* node = new Huffman::Node((uint8_t)i, freq[i]);
                    pq.push(node);
                }
            }

            // Handle single unique byte: wrap in an internal node
            // so the tree has at least one branch (code = "0")
            if (pq.size() == 1) {
                Huffman::Node* nd = pq.top();
                pq.pop();
                Huffman::Node* parent = new Huffman::Node(nd->freq, nd, nullptr);
                pq.push(parent);
            }

            while (pq.size() > 1) {
                Huffman::Node* child1 = pq.top();
                pq.pop();

                Huffman::Node* child2 = pq.top();
                pq.pop();

                uint64_t parent_freq = child1->freq + child2->freq;
                Huffman::Node* parent = new Huffman::Node(parent_freq, child1, child2);
                pq.push(parent);
            }

            Huffman::Node* root = pq.top();

            std::unordered_map<uint8_t, std::string> codeTable;
            if (root) Huffman::buildCodeTable(root, "", codeTable);

            uint64_t comp_size_pos = out.tellp();
            uint64_t placeholder = 0;
            out.write((char*)&placeholder, 8);

            uint64_t start_pos = out.tellp();
            BitWriter bw(out);
            
            Huffman::serializeTree(root, bw);

            for (uint8_t byte : content) {
                std::string code = codeTable[byte];
                for (char c : code) bw.writeBit(c - '0');
            }
            bw.flush();

            uint64_t end_pos = out.tellp();
            uint64_t comp_size = end_pos - start_pos;

            out.seekp(comp_size_pos);
            out.write((char*)&comp_size, 8);
            out.seekp(end_pos);

            delete root;
        }
        std::cout << "Archive created: " << archiveName << std::endl;
    }

    void extractArchive(const std::string& archiveName) {
        std::ifstream in(archiveName, std::ios::binary);
        if (!in) {
            std::cerr << "Error reading archive " << archiveName << std::endl;
            return;
        }

        char magic[5];
        in.read(magic, 5);
        if (!in) {
            std::cerr << "[!]TOo small" << std::endl;
            return;
        }
        if (std::memcmp(magic, "CARCH", 5) != 0) {
            std::cerr << "[!] NOT A CARCH FILE!~" << std::endl;
            return;
        }
        uint32_t file_count;
        in.read((char*)&file_count, 4);

        for (uint32_t i = 0; i < file_count; ++i) {
            std::string fname;
            char c;
            while (in.get(c) && c != '\0') fname += c;

            uint64_t orig_size;
            in.read((char*)&orig_size, 8);

            // CRC32 checksum value for the original file
            uint32_t crc32_input;
            in.read((char*)&crc32_input, 4);

            uint64_t comp_size;
            in.read((char*)&comp_size, 8);

            uint64_t data_start_pos = in.tellg();

            // Build output filename with _decompressed before the extension
            std::string outname;
            auto dot = fname.rfind('.');
            if (dot != std::string::npos)
                outname = fname.substr(0, dot) + "_decompressed" + fname.substr(dot);
            else
                outname = fname + "_decompressed";
            
            std::ofstream out(outname, std::ios::binary);
            if (out && orig_size > 0) {
                    //ADD : code the extracting text part from deserialized tree
                    BitReader br(in);

                    Huffman::Node* root = Huffman::deserializeTree(br);

                    // Buffer to store the uncompressed file in bytes
                    std::vector<uint8_t> buffer(orig_size);

                    if (root) {
                        uint64_t decoded_count = 0;
            
                        while (decoded_count < orig_size) {
                            Huffman::Node* current = root;
                    
                            while (current->left || current->right) {
                                int bit = br.readBit();
                                if (bit == 0) 
                                current = current->left;
                                else          
                                current = current->right;
                            }
                            buffer[decoded_count] = current->symbol;
                            decoded_count++;
                        }

                        delete root;
                    }

                    // Write the buffer to disk
                    out.write(reinterpret_cast<const char*>(buffer.data()), orig_size);

                    // CRC32 Checksum for the uncompressed file
                    uint32_t crc32_output = generate_checksum(buffer);
                    
                    // Checks whether the compression and decompression was successful or not
                    std::cout << "CRC32 (original):  0x" << std::hex << crc32_input << std::dec << std::endl;
                    std::cout << "CRC32 (extracted): 0x" << std::hex << crc32_output << std::dec << std::endl;
                    if (crc32_input == crc32_output) {
                        std::cout << "Extracted Successfully: " << outname << std::endl;
                    }
                    else {
                        std::cout << "CRC32 checksum FAILED for " << outname << std::endl;
                    }
            } else if (orig_size == 0) {
                std::cout << "Extracted (empty): " << outname << std::endl;
            } else {
                std::cerr << "Cannot open " << fname << std::endl;
            }

            in.seekg(data_start_pos + comp_size, std::ios::beg);
        }
    }
}
