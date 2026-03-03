make
./carch c myarchive.carch myfile1.txt exampleFile2.bmp
./carch x original.carch

If you don't want your original files overridden while unpacking and want to **extract resources back safely under brand new directories**:
Create a temporary directory, drop the `.carch` component in, change locations, and invoke extraction cleanly without colliding conflicts! Follow these safety steps: 

```bash
mkdir extr_test
mv myarchive.carch extr_test/
cd extr_test
../carch x myarchive.carch
```
