//
// Created by 11387 on 2023/8/24.
//
#include <stdio.h>
#include "delHiddenFiles.h"
// gcc -o go_del dstring.c delHiddenFiles.c main.c

int main() {
    char buffer[MAX_PATH];					// ��ŵ�ǰ��·��
    GetCurrentDirectory(MAX_PATH, buffer); 	// ��ȡ��ǰ·��
    initDelData();

    listFiles(buffer);
    return 0;
}