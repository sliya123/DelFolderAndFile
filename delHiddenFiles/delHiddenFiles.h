//
// Created by 11387 on 2023/8/24.
//

#ifndef DELHIDDENFILES_DELHIDDENFILES_H
#define DELHIDDENFILES_DELHIDDENFILES_H

#include <windows.h>
#include <stdbool.h>

void initDelData();

/**
 * 删除传入的文件夹，文件夹可以为空文件夹或者非空文件夹
 * @param lpszFolderPath 删除文件夹的路径
 * LPCTSTR是Windows API中常用的字符串类型参数,它代表一个指向常量字符串的指针(Pointer to Constant TCHAR String)。
 * @return
 */
bool deleteFolder(LPCTSTR lpszFolderPath);

/**
 * 迭代遍历文件夹
 * @param basePath
 */
void listFiles(const char *basePath);

#endif //DELHIDDENFILES_DELHIDDENFILES_H
