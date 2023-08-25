# DelFolderAndFile
根据文件名删除文件或文件

## 灵感来源：
makbook压缩包在Windows平台解压后会有一些无用的文件，这些文件在每个文件夹中都存在，为了删除这些文件，开发此程序

## 如何使用？
打开data.txt文件，在 del_folder: 下面写入你想要删除的文件夹名，在 del_file: 下写入想要删除的文件。
注意格式，一定要写在下面，且行首和行尾不能有空格，后续待优化。

程序默认删除当前路径下的文件夹及其子文件夹中的文件。

# 编译程序
```
mkdir build
cd build
cmake -G"MinGW Makefiles" ..
cmake --build .
```
