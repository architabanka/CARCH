#include "archive.hpp"
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

            uint64_t freq[256] = {0};
            //ADD : increment frequency for each character in ascii
            for(auto val:content) {
                freq[val]++;
            }

            out.write((char*)freq, sizeof(freq));

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

            uint64_t freq[256];
            in.read((char*)freq, sizeof(freq));

            uint64_t comp_size;
            in.read((char*)&comp_size, 8);

            uint64_t data_start_pos = in.tellg();

            std::ofstream out(fname, std::ios::binary);
            if (out && orig_size > 0) {
               //ADD : code the extracting text part from deserialized tree
                BitReader br(in);

                Huffman::Node* root = Huffman::deserializeTree(br);

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
                out.put(current->symbol);
                decoded_count++;
                }

                delete root;
                }
                std::cout << "Extracted: " << fname << std::endl;
            } else if (orig_size == 0) {
                std::cout << "Extracted (empty): " << fname << std::endl;
            } else {
                std::cerr << "Cannot open " << fname << std::endl;
            }

            in.seekg(data_start_pos + comp_size, std::ios::beg);
        }
    }
}
