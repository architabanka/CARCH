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
            encrypt(archiveName, password);
        } else if (cmd == "x") {
            string tempName = "kughfiufxn";
            string password;
            cin >> password;
            decrypt(archiveName, tempName, password);
            Archive::extractArchive(tempName);
            if (!filesystem::remove(tempName)) {
                cout<<"Dont delete the file"<< tempName << "while the process is going on.";
            } 
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
