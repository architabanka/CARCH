#include <iostream>
#include <vector>
#include <string>
#include "archive.hpp"

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
        } else if (cmd == "x") {
            Archive::extractArchive(archiveName);
        } else {
            cerr << "Unknown command: " << cmd << endl;
            return 1;
        }
    } catch (const exception& e) {
        cerr << "Exception: " << e.what() << endl;
        return 1;
    }

    return 0;
}
