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
        return 1;
    }

    string cmd = argv[1];
    string archiveName = argv[2];

    try {
        if (cmd == "c") {
            vector<string> fileNames;
            for (int i = 3; i < argc; ++i) {
                fileNames.push_back(argv[i]);
            }
            Archive::createArchive(archiveName, fileNames);
            string password;
            cin >> password;
            if (encrypt(archiveName, password) != 0) {
                cerr << "Error in encryption";
                return -1;
            }
        } else if (cmd == "x") {
            string tempName = "_carch_" + archiveName + ".txt";
            string password;
            cin >> password;
            if (decrypt(archiveName, tempName, password) != 0) {
                cerr << "Error in decryption";
                return -1;
            }
            Archive::extractArchive(tempName);
            // if (!filesystem::remove(tempName)) {
            //    cout<<"Dont delete the file"<< tempName << "while the process is going on.";
            // } 
        }
        else {
            cerr << "Unknown command: " << cmd << endl;
            return 1;
        }
    } catch (const exception& e) {
        cerr << "Exception: " << e.what() << endl;
        return 1;
    }

    return 0;
}
