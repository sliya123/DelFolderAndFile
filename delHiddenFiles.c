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
    // folder_s Ϊ Ҫɾ�����ļ��У�file_s Ϊ Ҫɾ�����ļ�
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
            buff[strlen(buff) - 1] = '\0'; // fgets �Ὣ�س������� buff
            ds_add_str(file_ps, buff);
            ds_add_str(file_ps, Separator);
        }

        if (strstr(buff, "del_folder:"))
            folder_flag = true;
        if (strstr(buff, "del_file:"))
            file_flag = true;
    }
    fclose(fp); // �ر��ļ���

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

//    �ͷŶ�̬�ַ���
    free_ds(folder_ps);
    free_ds(file_ps);
}

bool deleteFolder(LPCTSTR lpszFolderPath) {
    // �����ļ������ļ���
    HANDLE hFind;
    WIN32_FIND_DATA fd;

    TCHAR szFullPath[MAX_PATH];
    lstrcpy(szFullPath, lpszFolderPath); // �����ļ���·��
    lstrcat(szFullPath, TEXT("\\*")); // ����ͨ���*

    hFind = FindFirstFile(szFullPath, &fd); // ���ҵ�һ���ļ�/�ļ���
    if (hFind == INVALID_HANDLE_VALUE) return FALSE; // ����ʧ���򷵻�

    do {
        if (lstrcmp(fd.cFileName, TEXT(".")) != 0 && lstrcmp(fd.cFileName, TEXT("..")) != 0) { // �ų���ǰĿ¼�͸�Ŀ¼
            lstrcpy(szFullPath, lpszFolderPath); // �����ļ���·��
            lstrcat(szFullPath, TEXT("\\"));
            lstrcat(szFullPath, fd.cFileName); // �����ҵ����ļ���

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // �ҵ������ļ���,�ݹ�ɾ��
                deleteFolder(szFullPath);
            } else {
                // �ҵ������ļ�,ֱ��ɾ��
                DeleteFile(szFullPath);
            }
        }
    } while (FindNextFile(hFind, &fd)); // ������һ���ļ�/�ļ���

    FindClose(hFind); // �رղ��Ҿ��
    // ɾ�����ļ���
    return RemoveDirectory(lpszFolderPath);
}

void listFiles(const char *basePath) {
	
    char searchPath[MAX_PATH];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", basePath); // ��������·�������ڱ���Ŀ¼

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath, &findFileData); // �򿪵�һ���ļ�/�ļ���

    if (hFind == INVALID_HANDLE_VALUE) {
        perror("FindFirstFile"); // ����޷��򿪣��������
        return;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
            char absolutePath[MAX_PATH];
            snprintf(absolutePath, sizeof(absolutePath), "%s\\%s", basePath, findFileData.cFileName);
            printf("absolutePath = %s\n", absolutePath);
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { // �ж��Ƿ�Ϊ�ļ���
                printf("�ļ��У�%s\n", findFileData.cFileName); // ����ļ�����
                fflush(stdout); // ˢ�±�׼���������
                for (int i = 0; i < del_folders_size; ++i) {
                    if (strcmp(findFileData.cFileName, del_folders[i]) == 0) {
                        if (deleteFolder(absolutePath))
                            printf("�ļ��У�%s ɾ���ɹ�", findFileData.cFileName);
                        else
                            printf("�ļ��У�%s ɾ��ʧ��", findFileData.cFileName);
						break;
                    } else {
                        listFiles(absolutePath);
                    }
                }
            } else {
                printf("�ļ���%s\n", findFileData.cFileName); // ����ļ���
                for (int i = 0; i < del_files_size; ++i) {
                    if (strcmp(findFileData.cFileName, del_files[i]) == 0) {
                        DeleteFile(absolutePath);
                        printf("�ļ� %s ɾ���ɹ�\n", findFileData.cFileName);
						break;
                    }
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0); // ������һ���ļ�/�ļ���

    if (GetLastError() != ERROR_NO_MORE_FILES) {
        perror("FindNextFile"); // ����ڱ��������г��ִ����������
    }

    FindClose(hFind); // �ر��ļ����Ҿ��
}


