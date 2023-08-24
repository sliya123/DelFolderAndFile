//
// Created by 11387 on 2023/8/24.
//
#include <stdio.h>
#include "dstring.h"
#include "delHiddenFiles.h"

#define BUFF_SIZE 1024
#define MAX_SIZE 100
#define Separator ","

char *del_files[MAX_SIZE];
int del_files_size;
char *del_folders[MAX_SIZE];
int del_folders_size;

void initDelData() {
    FILE *fp = fopen("./data.txt", "r");
    bool folder_flag = false;
    bool file_flag = false;
    char buff[BUFF_SIZE];
    // folder_s 为 要删除的文件夹，file_s 为 要删除的文件
    char *folder_s, *file_s;
    char **folder_ps = &folder_s;
    folder_s = new_ds(0);
    char **file_ps = &file_s;
    file_s = new_ds(0);

    while (fgets(buff, BUFF_SIZE, fp) != NULL) {
        bool buff_flag = false;
        size_t len = strlen(buff);
        for (int i = 0; i < len; ++i) {
            if (buff[i] != ' ' && buff[i] != '\t' && buff[i] != '\n' && buff[i] != '\r') {
                buff_flag = true;
                break;
            }
        }
        if (buff_flag && folder_flag && !file_flag && strstr(buff, "del_file:") == 0) {
            buff[strlen(buff) - 1] = '\0';
            ds_add_str(folder_ps, buff);
            ds_add_str(folder_ps, Separator);
        }

        if (buff_flag && file_flag) {
            buff[strlen(buff) - 1] = '\0'; // fgets 会将回车符赋给 buff
            ds_add_str(file_ps, buff);
            ds_add_str(file_ps, Separator);
        }

        if (strstr(buff, "del_folder:"))
            folder_flag = true;
        if (strstr(buff, "del_file:"))
            file_flag = true;
    }
    fclose(fp); // 关闭文件流

    char *del_folder = strtok(folder_s, Separator);
    int i1 = 0;
    while (del_folder != NULL) {
        del_folders[i1++] = del_folder;
        del_folder = strtok(NULL, Separator);
    }
    del_folders_size = i1;

    char *del_file = strtok(file_s, Separator);
    int i2 = 0;
    while (del_file != NULL) {
        del_files[i2++] = del_file;
        del_file = strtok(NULL, Separator);
    }
    del_files_size = i2;

//    释放动态字符串
    free_ds(folder_ps);
    free_ds(file_ps);
}

bool deleteFolder(LPCTSTR lpszFolderPath) {
    // 查找文件及子文件夹
    HANDLE hFind;
    WIN32_FIND_DATA fd;

    TCHAR szFullPath[MAX_PATH];
    lstrcpy(szFullPath, lpszFolderPath); // 拷贝文件夹路径
    lstrcat(szFullPath, TEXT("\\*")); // 加上通配符*

    hFind = FindFirstFile(szFullPath, &fd); // 查找第一个文件/文件夹
    if (hFind == INVALID_HANDLE_VALUE) return FALSE; // 查找失败则返回

    do {
        if (lstrcmp(fd.cFileName, TEXT(".")) != 0 && lstrcmp(fd.cFileName, TEXT("..")) != 0) { // 排除当前目录和父目录
            lstrcpy(szFullPath, lpszFolderPath); // 拷贝文件夹路径
            lstrcat(szFullPath, TEXT("\\"));
            lstrcat(szFullPath, fd.cFileName); // 加上找到的文件名

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // 找到的是文件夹,递归删除
                deleteFolder(szFullPath);
            } else {
                // 找到的是文件,直接删除
                DeleteFile(szFullPath);
            }
        }
    } while (FindNextFile(hFind, &fd)); // 查找下一个文件/文件夹

    FindClose(hFind); // 关闭查找句柄
    // 删除空文件夹
    return RemoveDirectory(lpszFolderPath);
}

void listFiles(const char *basePath) {
	
    char searchPath[MAX_PATH];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", basePath); // 构建搜索路径，用于遍历目录

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath, &findFileData); // 打开第一个文件/文件夹

    if (hFind == INVALID_HANDLE_VALUE) {
        perror("FindFirstFile"); // 如果无法打开，输出错误
        return;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
            char absolutePath[MAX_PATH];
            snprintf(absolutePath, sizeof(absolutePath), "%s\\%s", basePath, findFileData.cFileName);
            printf("absolutePath = %s\n", absolutePath);
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { // 判断是否为文件夹
                printf("文件夹：%s\n", findFileData.cFileName); // 输出文件夹名
                fflush(stdout); // 刷新标准输出缓冲区
                for (int i = 0; i < del_folders_size; ++i) {
                    if (strcmp(findFileData.cFileName, del_folders[i]) == 0) {
                        if (deleteFolder(absolutePath))
                            printf("文件夹：%s 删除成功", findFileData.cFileName);
                        else
                            printf("文件夹：%s 删除失败", findFileData.cFileName);
						break;
                    } else {
                        listFiles(absolutePath);
                    }
                }
            } else {
                printf("文件：%s\n", findFileData.cFileName); // 输出文件名
                for (int i = 0; i < del_files_size; ++i) {
                    if (strcmp(findFileData.cFileName, del_files[i]) == 0) {
                        DeleteFile(absolutePath);
                        printf("文件 %s 删除成功\n", findFileData.cFileName);
						break;
                    }
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0); // 遍历下一个文件/文件夹

    if (GetLastError() != ERROR_NO_MORE_FILES) {
        perror("FindNextFile"); // 如果在遍历过程中出现错误，输出错误
    }

    FindClose(hFind); // 关闭文件查找句柄
}


