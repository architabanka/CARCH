#pragma once
#include<string>

std::vector<unsigned char> salt_gen();
std::vector<uint32_t> key_gen(const std::string& password, const std::vector<unsigned char>& salt);
std::vector<uint32_t> round_key_gen(const std::vector<uint32_t>& key);

int encrypt(const std::string& archiveName, const std::string& password);
int decrypt(const std::string& sourcePath, const std::string& destPath,const std::string& password);