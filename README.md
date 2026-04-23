To use the archive manager run the following commands upon clone the repo 
```bash
# Build
make

# Create Archive
./carch c myarchive.carch myfile1.txt exampleFile2.bmp

# Extract Archive
./carch x myarchive.carch

```
The archive manager creates compressed version of files with the extension .carch which is about 70% of the original file size. 
