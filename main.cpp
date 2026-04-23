#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include "archive.hpp"
#include "aes.hpp"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Usage:" << endl;
        cerr << "  " << argv[0] << " c <archive.carch> <file1> <file2> ..." << endl;
        cerr << "  " << argv[0] << " x <archive.carch>" << endl;
        cerr << "  " << argv[0] << " e <fileName>" <<endl;
        cerr << "  " << argv[0] << " d <fileName>" <<endl;
        return 1;
    }

    string cmd = argv[1];
    string archiveName = argv[2];

    try {
        if (cmd == "c") {
            vector<string> fileNames;
            unsigned long long orig_size = 0;
            for (int i = 3; i < argc; ++i) {
                fileNames.push_back(argv[i]);
                auto temp = std::filesystem::file_size(argv[i]);
                orig_size += temp;
            }
            Archive::createArchive(archiveName, fileNames);
            string password;
            cout << "Create a Password: ";
            cin >> password;
            if (encrypt(archiveName, password) != 0) {
                cerr << "Error in encryption";
                return -1;
            }
            auto size = std::filesystem::file_size(archiveName);
            cout << " Compressed Archive size: " << size/(1024.0 * 1024.0) << " MB\n";
            cout << " Memory Saved: " << (orig_size - size) / (1024.0 * 1024.0) << " MB\n";
        } else if (cmd == "x") {
            string tempName = "Decrypted_" + archiveName;
            string password;
            cout << "Enter the Password: ";
            cin >> password;
            if (decrypt(archiveName, tempName, password) != 0) {
                cerr << "Error in decryption";
                return -1;
            }
            Archive::extractArchive(tempName);
            if (!filesystem::remove(tempName)) {
                cout<<"Dont delete the file"<< tempName << "while the process is going on.";
            }
        } else if (cmd == "e") {
            string password;
            cout << "Create a Password: ";
            cin >> password;
            if (encrypt(archiveName, password) != 0) {
                cerr << "Error in encryption";
                return -1;
            }
        } else if (cmd == "d") {
            string password;
            cout << "Enter the Password: ";
            cin >> password;
            if (decrypt(archiveName, "carch_decrypted_" + archiveName, password) != 0) {
                cerr << "Error in decryption";
                return -1;
            }
        } else {
            cerr << "Error: Unknown Command" << endl;
            cerr << "Usage:" << endl;
            cerr << "  " << argv[0] << " c <archive.carch> <file1> <file2> ..." << endl;
            cerr << "  " << argv[0] << " x <archive.carch>" << endl;
            cerr << "  " << argv[0] << " e <fileName>" <<endl;
            cerr << "  " << argv[0] << " d <fileName>" <<endl;
            return -1;
        }
    } 
    catch (const exception& e) {
        cerr << "Exception: " << e.what() << endl;
        return -1;
    }

    return 0;
}
