//
// Created by 11387 on 2023/8/24.
//

#ifndef DELHIDDENFILES_DELHIDDENFILES_H
#define DELHIDDENFILES_DELHIDDENFILES_H

#include <windows.h>
#include <stdbool.h>

void initDelData();

/**
 * ɾ��������ļ��У��ļ��п���Ϊ���ļ��л��߷ǿ��ļ���
 * @param lpszFolderPath ɾ���ļ��е�·��
 * LPCTSTR��Windows API�г��õ��ַ������Ͳ���,������һ��ָ�����ַ�����ָ��(Pointer to Constant TCHAR String)��
 * @return
 */
bool deleteFolder(LPCTSTR lpszFolderPath);

/**
 * ���������ļ���
 * @param basePath
 */
void listFiles(const char *basePath);

#endif //DELHIDDENFILES_DELHIDDENFILES_H
