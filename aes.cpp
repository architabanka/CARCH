#include <openssl/evp.h>
#include <openssl/rand.h>

#include <filesystem>
#include <fstream>
#include <iostream>

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include "aes.hpp"


std::vector<unsigned char> salt_gen() {
    std::vector<unsigned char> salt(16);
    if (RAND_bytes(salt.data(), 16) != 1) {
        throw std::runtime_error("Error generating random salt using OpenSSL");
    }
    return salt;
}

std::vector<uint32_t> key_gen(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<uint32_t> key;
    std::vector<unsigned char> out_key;

    const int iterations = 100000;
    out_key.resize(16);

    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(), 
        password.length(), 
        salt.data(), 
        salt.size(), 
        iterations, 
        EVP_sha256(), // Use SHA-256 for the underlying hash
        16, 
        out_key.data()
    );
    if (result != 1) {
        throw std::runtime_error("Error generating AES key from password using OpenSSL");
    }

    key.resize(4);
    std::memcpy(key.data(), out_key.data(), 16);
    return key;
}

std::vector<uint32_t> round_key_gen(const std::vector<uint32_t>& key) {
    std::vector<uint32_t> round_keys(44, 0);

    for (int i = 0; i < 4; i++) {
        round_keys[i] = key[i];
    }

    for (int i = 4; i < 44; i++) {
        uint32_t temp = round_keys[i - 1];

        if (i % 4 == 0) {
            // 1. RotWord
            temp = (temp << 8) | (temp >> 24);

            // 2. SubWord (extract bytes, apply S-box, reassemble)
            uint8_t b1 = sbox[(temp >> 24) & 0xFF];
            uint8_t b2 = sbox[(temp >> 16) & 0xFF];
            uint8_t b3 = sbox[(temp >> 8) & 0xFF];
            uint8_t b4 = sbox[temp & 0xFF];

            temp = (static_cast<uint32_t>(b1) << 24) | 
                   (static_cast<uint32_t>(b2) << 16) | 
                   (static_cast<uint32_t>(b3) << 8)  | 
                    static_cast<uint32_t>(b4);

            temp ^= rcon[(i / 4) - 1];
        }

        round_keys[i] = round_keys[i - 4] ^ temp;
    }
    return round_keys;
}

int encrypt(const std::string& archiveName, const std::string& password) {
    std::fstream file(archiveName, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Archive not Found." << std::endl;
        return -1;
    }
    // PKCS#7 Method Padding
    // The way we convert the size into multiple of 16 bytes is add 01 as a padding 
    // if we want 1 byte (Note: if it is already multiple of 16 then we pad it with 16 bytes)
    auto fileSize = static_cast<std::uintmax_t>(file.tellg());
    uint8_t padding = 16 - fileSize % 16;

    std::vector<char> myData(padding);
    for (int i = 0; i < padding; i++) {
        myData[i] = padding;
    }
    file.write(myData.data(), padding);

    std::vector<unsigned char> salt = salt_gen();
    std::vector<uint32_t> key = key_gen(password, salt);
    std::vector<uint32_t> round_keys = round_key_gen(key);

    file.clear();
    file.seekg(0, std::ios::beg);
    file.seekp(0, std::ios::beg);

    uint8_t buffer[16];
    std::streampos current_pos = 0;

    while (true) {
        file.seekg(current_pos);
        file.read(reinterpret_cast<char*>(buffer), 16);
        if (file.gcount() != 16) break;

        uint8_t state[4][4];
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                state[row][col] = buffer[4 * col + row];
            }
        }

        // Add AES key
        for (int col = 0; col < 4; col++) {
            uint32_t word =
                ((uint32_t)state[0][col] << 24) |
                ((uint32_t)state[1][col] << 16) |
                ((uint32_t)state[2][col] << 8)  |
                ((uint32_t)state[3][col]);

            word ^= round_keys[col];

            state[0][col] = (word >> 24) & 0xFF;
            state[1][col] = (word >> 16) & 0xFF;
            state[2][col] = (word >> 8)  & 0xFF;
            state[3][col] =  word        & 0xFF;
        }

        for (int round = 0; round < 10; round++) {
            // Step 1: Byte sub
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    state[i][j] = sbox[state[i][j]];
                }
            }

            // Step 2: Row Shifting (Left Circular)
            uint8_t temp[4][4];
            for (int row = 0; row < 4; row++) {
                for (int col = 0; col < 4; col++) {
                    temp[row][col] = state[row][(col + row) % 4];
                }
            }
            for (int row = 0; row < 4; row++) {
                for (int col = 0; col < 4; col++) {
                    state[row][col] = temp[row][col];
                }
            }

            // Step 3: Mix Columns (Skipped in last round)
            if (round != 9) {
                for (int col = 0; col < 4; col++) {
                    uint8_t s0 = state[0][col];
                    uint8_t s1 = state[1][col];
                    uint8_t s2 = state[2][col];
                    uint8_t s3 = state[3][col];

                    temp[0][col] = mul_2[s0] ^ mul_3[s1] ^ s2 ^ s3;
                    temp[1][col] = s0 ^ mul_2[s1] ^ mul_3[s2] ^ s3;
                    temp[2][col] = s0 ^ s1 ^ mul_2[s2] ^ mul_3[s3];
                    temp[3][col] = mul_3[s0] ^ s1 ^ s2 ^ mul_2[s3];
                }
                std::memcpy(state, temp, 16);
            }

            // Step 4: Add Round key
            int base_idx = (round + 1) * 4;
            for (int col = 0; col < 4; col++) {
                uint32_t word =
                    ((uint32_t)state[0][col] << 24) |
                    ((uint32_t)state[1][col] << 16) |
                    ((uint32_t)state[2][col] << 8)  |
                    ((uint32_t)state[3][col]);

                word ^= round_keys[base_idx + col];

                state[0][col] = (word >> 24) & 0xFF;
                state[1][col] = (word >> 16) & 0xFF;
                state[2][col] = (word >> 8)  & 0xFF;
                state[3][col] =  word        & 0xFF;
            }
        }

        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                buffer[4 * col + row] = state[row][col];
            }
        }

        file.clear();
        file.seekp(current_pos);
        file.write(reinterpret_cast<char*>(buffer), 16);
        current_pos += 16;
    }

    file.clear();
    
    // salt at end
    std::vector<char> temp_salt(salt.begin(), salt.end());
    file.seekp(0, std::ios::end);
    file.write(temp_salt.data(), 16);
    file.close();
    return 0;
}


int decrypt(const std::string& sourcePath, const std::string& destPath, const std::string& password) {
    std::filesystem::copy_file(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing);
    std::ifstream ifile(destPath, std::ios::in | std::ios::binary);

    if (!ifile) {
        std::cerr << "Archive not Found." << std::endl;
        return -1;
    }

    auto fileSize = std::filesystem::file_size(destPath);
    if ((int)fileSize <= 16) {
        std::cerr << "Too small to be a valid archive." << std::endl;
        ifile.close();
        return -1;
    }
    if (fileSize % 16 != 0) {
        std::cerr << "Decompressed file is invalid. File size is not a multiple of 16 bytes." << std::endl;
        ifile.close();
        return -1;
    }

    ifile.seekg(-16, std::ios::end);
    char temp_salt[16];
    ifile.read(temp_salt, 16);
    ifile.close();
    std::filesystem::resize_file(destPath, fileSize - 16);

    std::vector<unsigned char> salt(temp_salt, temp_salt + 16);
    std::vector<uint32_t> key = key_gen(password, salt);
    std::vector<uint32_t> round_keys = round_key_gen(key);

    std::fstream file(destPath, std::ios::in | std::ios::out | std::ios::binary);

    file.seekg(0, std::ios::beg);
    file.seekp(0, std::ios::beg);

    uint8_t buffer[16];
    std::streampos current_pos = 0; // Track position manually

    while (true) {
        file.seekg(current_pos);
        file.read(reinterpret_cast<char*>(buffer), 16);
        if (file.gcount() != 16) break;

        uint8_t state[4][4];
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                state[row][col] = buffer[4 * col + row];
            }
        }

        for (int round = 9; round >= 0; round--) {
            // Step 1: Add round key
            int base_idx = (round + 1) * 4;
            for (int col = 0; col < 4; col++) {
                uint32_t word =
                    ((uint32_t)state[0][col] << 24) |
                    ((uint32_t)state[1][col] << 16) |
                    ((uint32_t)state[2][col] << 8)  |
                    ((uint32_t)state[3][col]);

                word ^= round_keys[base_idx + col];

                state[0][col] = (word >> 24) & 0xFF;
                state[1][col] = (word >> 16) & 0xFF;
                state[2][col] = (word >> 8)  & 0xFF;
                state[3][col] =  word        & 0xFF;
            }
            
            // Step 2: Undo Mix Columns (Skipped in first round)
            uint8_t temp[4][4];
            if (round != 9) {
                for (int col = 0; col < 4; col++) {
                    uint8_t s0 = state[0][col];
                    uint8_t s1 = state[1][col];
                    uint8_t s2 = state[2][col];
                    uint8_t s3 = state[3][col];
                    temp[0][col] = mul_14[s0] ^ mul_11[s1] ^ mul_13[s2] ^  mul_9[s3];
                    temp[1][col] =  mul_9[s0] ^ mul_14[s1] ^ mul_11[s2] ^ mul_13[s3];
                    temp[2][col] = mul_13[s0] ^  mul_9[s1] ^ mul_14[s2] ^ mul_11[s3];
                    temp[3][col] = mul_11[s0] ^ mul_13[s1] ^  mul_9[s2] ^ mul_14[s3];
                }
                std::memcpy(state, temp, 16);
            }

            // Step 3: Row Shifting (Right Circular)
            for (int row = 0; row < 4; row++) {
                for (int col = 0; col < 4; col++) {
                    temp[row][col] = state[row][((col - row) % 4 + 4) % 4];
                }
            }
            for (int row = 0; row < 4; row++) {
                for (int col = 0; col < 4; col++) {
                    state[row][col] = temp[row][col];
                }
            }

            // Step 4: Byte substitution
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    state[i][j] = inv_sbox[state[i][j]];
                }
            }
        }

        // Undo AES Key added
        for (int col = 0; col < 4; col++) {
            uint32_t word =
                ((uint32_t)state[0][col] << 24) |
                ((uint32_t)state[1][col] << 16) |
                ((uint32_t)state[2][col] << 8)  |
                ((uint32_t)state[3][col]);

            word ^= round_keys[col];

            state[0][col] = (word >> 24) & 0xFF;
            state[1][col] = (word >> 16) & 0xFF;
            state[2][col] = (word >> 8)  & 0xFF;
            state[3][col] =  word        & 0xFF;
        }

        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                buffer[4 * col + row] = state[row][col];
            }
        }
        
        file.clear();
        file.seekp(current_pos);
        file.write(reinterpret_cast<char*>(buffer), 16);
        current_pos += 16;
    }

    file.clear();
    file.seekg(-1, std::ios::end);
    uint8_t pad_len;
    file.read(reinterpret_cast<char*>(&pad_len), 1);

    if (pad_len == 0 || pad_len > 16) {
        std::cerr << "Decompressed file is invalid. Verify your password." << std::endl;
        file.close();
        return -1;
    }

    file.clear();
    file.seekg(-static_cast<std::streamoff>(pad_len), std::ios::end);

    for (int i = 0; i < pad_len; i++) {
        uint8_t padding;
        file.read(reinterpret_cast<char*>(&padding), 1);
        if (pad_len != padding) {
            std::cerr << "Decompressed file is invalid. Verify your password." << std::endl;
            file.close();
            return -1;
        }
    }
    file.close();
    std::filesystem::resize_file(destPath, fileSize - 16 - pad_len);
    return 0;
}