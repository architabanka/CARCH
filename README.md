# Carch Archive Manager

A C++ archive manager that uses Huffman compression, AES encryption and CRC-32 CheckSum.

## Prerequisites

This project requires **OpenSSL**.

### macOS
Install OpenSSL via [Homebrew](https://brew.sh/):
```bash
brew install openssl@3
```

### Linux
Install OpenSSL development headers (e.g., on Ubuntu):
```bash
sudo apt-get install libssl-dev
```

## How to use
Run the following commands after cloning the repo:
```bash
# go inside src folder
cd src

# Build
make

# Create Archive
./carch c myarchive.carch myfile1.txt exampleFile2.bmp

# Extract Archive
./carch x myarchive.carch

```
The archive manager creates compressed version of files with the extension .carch and name filename_decompressed.file_extension in the same directory as the original file which is about 60-70% of the original file size. To compress and extract the file(s) it requires the user to enter a password ensuring security. A crc 32 Checksum value of the original file that was compressed and of the new decompressed version is displayed to show the suer no data of the file was tampered during the process.
Currently the archive supports compression of text files.

