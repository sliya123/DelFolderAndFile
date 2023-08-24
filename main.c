//
// Created by 11387 on 2023/8/24.
//
#include <stdio.h>
#include "delHiddenFiles.h"
// gcc -o go_del dstring.c delHiddenFiles.c main.c

int main() {
    char buffer[MAX_PATH];					// 存放当前的路径
    GetCurrentDirectory(MAX_PATH, buffer); 	// 获取当前路径
    initDelData();

    listFiles(buffer);
    return 0;
}