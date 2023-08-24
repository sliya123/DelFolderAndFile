#include <stdio.h>   // perror()
#include <stdlib.h>  // malloc()  realloc()  exit()
#include <stdarg.h>  // va_start()  va_arg()  va_end()
#include <string.h>  // memcpy()  strlen()

#include "dstring.h"

/* ============================================================
���ܣ�ʵ�ֶ�̬�ַ���������������������Ҫ�Զ���չ��
	  ��̬�ַ������Ե�����ͨ�ַ���ʹ�ã�����ͨ��������ͬ��

�ڴ沼�֣��ڴ�ռ�� = 32 + ���������� + 1����

 0 -  7����̬�ַ������ "\xDA\xDA""DSTR""\xDC\xDC"��ħ�����֣�
 8 - 15���Զ��壬�����ڴ�ű������͵�
24 - 31���ַ������ȣ��ֽڣ�
16 - 23�����������ȣ��ֽڣ�
32 -  n���������ռ�
n + 1  ��'\0' �ַ�����֤�������� '\0' ��β
============================================================ */

// ��̬�ַ����ṹ��
typedef struct {
	uint64_t magic;   // ħ������
	uint64_t custom;  // �Զ�������
	uint64_t len;     // �ַ������ȣ����������� \0��
	uint64_t cap;     // ���������ȣ����������� \0��
	char data[];      // ��������ʼ���� \0 ��β��
} dstr;

static const int DS_HEADER_SIZE = sizeof(uint64_t) * 4;   // ͷ����С
static const uint64_t MAGIC_NUMBER = 0xDCDC52545344DADA;  // ħ������

// ����������data���������ֶε� uint64_t ƫ����
static const int OFF_MAGIC	 = -4;
static const int OFF_CUSTOM	 = -3;
static const int OFF_LEN     = -2;
static const int OFF_CAP     = -1;


/* ------------------------------------------------------------
   dynamic string get header
   ���ܣ��Ӷ�̬�ַ���������λ����ת��ͷ��λ��
   ��������̬�ַ�������
   ���أ���̬�ַ����ṹ��ָ��
------------------------------------------------------------ */
static dstr * ds_header(char *s)
{
	dstr *ds = (dstr *)(s - DS_HEADER_SIZE);

	if (ds->magic != MAGIC_NUMBER)
		return NULL;
	else
		return ds;
}


/* ------------------------------------------------------------
   new dynamic string
   ���ܣ�������̬�ַ���
   ������ҪԤ���Ļ�������С
   ���أ���̬�ַ������������������������ݵĵ�ַ��
------------------------------------------------------------ */
char * new_ds(size_t cap)
{
	dstr *ds;

	// Ԥ��β���� '\0' �ֽ�
	ds = malloc(DS_HEADER_SIZE + cap + 1);
	if (ds == NULL)
	{
		perror("newds()->malloc()");
		exit(1);
	}

	ds->magic   = MAGIC_NUMBER;
	ds->custom  = 0;
	ds->cap     = cap;
	ds->len     = 0;
	// ��Ϊ�����ڴ�ʱ +1 ���ֽڣ����Ե����� cap Ϊ 0 ʱ����д������ֽ�
	ds->data[0] = '\0';

	return ds->data;
}


/* ------------------------------------------------------------
   free dynamic string
   ���ܣ��ͷŶ�̬�ַ��������� *ps ����Ϊ NULL
   ��������̬�ַ��������ĵ�ַ
   ���أ���
------------------------------------------------------------ */
void free_ds(char **ps)
{
	dstr * ds = ds_header(*ps);

	if (ds == NULL)
	{
		printf("freeds(): not a dynamic string");
		exit(1);
	}

	free(ds);
	*ps = NULL;
}


/* ------------------------------------------------------------
   is dynamic string
   ���ܣ��ж��ַ����Ƿ�Ϊ��̬�ַ���
   �������ַ�������
   ���أ��Ƿ��� 1�����Ƿ��� 0
------------------------------------------------------------ */
int is_ds(const char *s)
{
	return ((uint64_t *)s)[OFF_MAGIC] == MAGIC_NUMBER;
}


/* ------------------------------------------------------------
   dynamic string custom data
   ���ܣ���ȡ��̬�ַ������Զ�������
   ��������̬�ַ�������
   ���أ��Զ�������
------------------------------------------------------------ */
uint64_t ds_custom(const char *s)
{
	return ((uint64_t *)s)[OFF_CUSTOM];
}


/* ------------------------------------------------------------
   dynamic string custom data
   ���ܣ����ö�̬�ַ������Զ�������
   ��������̬�ַ�����������������
   ���أ���
------------------------------------------------------------ */
void ds_set_custom(char *s, uint64_t custom)
{
	((uint64_t *)s)[OFF_CUSTOM] = custom;
}


/* ------------------------------------------------------------
   dynamic string length
   ���ܣ���ȡ��̬�ַ����ĳ���
   ��������̬�ַ�������
   ���أ��ַ������ȣ�������β���� '\0'��
------------------------------------------------------------ */
uint64_t ds_len(const char *s)
{
	return ((uint64_t *)s)[OFF_LEN];
}

static inline void ds_set_len(char *s, uint64_t len)
{
	((uint64_t *)s)[OFF_LEN] = len;
	s[len] = '\0';
}


/* ------------------------------------------------------------
   dynamic string capacity
   ���ܣ���ȡ��̬�ַ����Ļ���������
   ��������̬�ַ�������
   ���أ�������������������β���� '\0'��
------------------------------------------------------------ */
uint64_t ds_cap(const char *s)
{
	return ((uint64_t *)s)[OFF_CAP];
}


static inline void ds_set_cap(char *s, uint64_t cap)
{
	((uint64_t *)s)[OFF_CAP] = cap;
	s[cap] = '\0';
}


/* ------------------------------------------------------------
   dynamic string last char
   ���ܣ���ȡ��̬�ַ��������һ���ַ�
   ��������̬�ַ�������
   ���أ����һ���ַ�������ַ���Ϊ�գ��򷵻� '\0'
------------------------------------------------------------ */
int ds_last(char *s)
{
	uint64_t len = ds_len(s);

	if (len == 0)
		return '\0';
	else
		return s[len - 1];
}


/* ------------------------------------------------------------
   dynamic string end position
   ���ܣ���ȡ��̬�ַ������һ���ַ�֮���λ��
   ��������̬�ַ�������
   ���أ�λ��ָ��
------------------------------------------------------------ */
char * ds_end(char *s)
{
	return s + ds_len(s);
}


/* ------------------------------------------------------------
   grow dynamic string
   ���ܣ����� needsize �ж��Ƿ���Ҫ���ݣ�������Ҫִ������
   ��������̬�ַ��������ĵ�ַ��Ҫ����Ŀ��ÿռ��С
   ���أ���̬�ַ�������
------------------------------------------------------------ */
// ʹ�� Fast Inverse Square Root �㷨������ƽ����
float fast_sqrt(float x)
{
	float xhalf = 0.5f * x;
	int i = *(int*)&x;               // ��ȡ�������Ķ�����λ
	i = 0x5f375a86 - (i >> 1);       // ������ʼ�²� y0
	x = *(float*)&i;                 // ��������λת���ظ���
	x = x * (1.5f - xhalf * x * x);  // ţ�ٲ��裬�ظ���߾���
	return 1/x;
}

char * ds_grow(char **ps, size_t needsize)
{
	char *s = *ps;

	uint64_t len = ds_len(s) + needsize;
	uint64_t cap = ds_cap(s);

	// �ռ��㹻����������
	if (len <= cap)
		return s;

	// �������ݴ�С������Ԥ�����ٿռ�
	if (len > 128*1024*1024)
	    // len ���� 128M����Ԥ�� 32M
		cap = len + 32*1024*1024;
	else if (len > 4*1024*1024)
		// len �� (4M,128M] ֮�䣬�����ƽ��������Ԥ��
		// ��Ԥ�� len*2 �𽥹��ɵ�Ԥ�� len/4
		// �� 2M ������ 128M ������ 8 ��
		// len ���� 255M���� FastSqrt �������
		cap = len + fast_sqrt(len * 8) * 1024;
	else if (len < 16)
		// ���� 16 �ֽ�
		cap = 16;
	else
		// С�� 4M������Ԥ��
		cap = len * 2;

	return ds_resize(ps, cap);
}


/* ------------------------------------------------------------
   resize dynamic string capacity
   ���ܣ�������̬�ַ��������������Զ�������������� '\0'��
   ��������̬�ַ��������ĵ�ַ���µ�����
   ���أ���̬�ַ�������
------------------------------------------------------------ */
char * ds_resize(char **ps, size_t cap)
{
	char *s = *ps;

	// ����β���� '\0' λ��
	s = realloc(s - DS_HEADER_SIZE, DS_HEADER_SIZE + cap + 1);
	if (s == NULL)
	{
		perror("resizeds()->realloc()");
		exit(1);
	}
	// ָ��������
	s += DS_HEADER_SIZE;

	// ��������������С��ԭ�����ַ������ȣ�������
	if (ds_len(s) > cap)
		// �������Ȳ����� \0
		ds_set_len(s, cap);

    // �������������� \0
	ds_set_cap(s, cap);

	*ps = s;

	return s;
}


/* ------------------------------------------------------------
   pack dynamic string
   ���ܣ�ѹ����̬�ַ������ͷ�Ԥ���ռ䣬ֻ�������ݲ��֣�
   ��������̬�ַ��������ĵ�ַ
   ���أ���̬�ַ�������
------------------------------------------------------------ */
char * ds_pack(char **ps)
{
	return ds_resize(ps, ds_len(*ps));
}


/* ------------------------------------------------------------
   dynamic string add char
   ���ܣ���̬�ַ�����׷���ַ�
   ��������̬�ַ��������ĵ�ַ��Ҫд����ַ�
   ���أ���̬�ַ�������
------------------------------------------------------------ */
char * ds_add_char(char **ps, char c)
{
    ds_grow(ps, 1);

    char *s = *ps;

    ds_end(s)[0] = c;
    // �������Ȳ����� \0
    ds_set_len(s, ds_len(s) + 1);

    return s;
}

/* ------------------------------------------------------------
   dynamic string add string
   ���ܣ���̬�ַ�����׷���ַ���
   ��������̬�ַ��������ĵ�ַ��Ҫд����ַ���
   ���أ���̬�ַ�������
------------------------------------------------------------ */
char * ds_add_str(char **ps, const char* sub)
{
	size_t len = strlen(sub);
    ds_grow(ps, len);

    char *s = *ps;

    memcpy(ds_end(s), sub, len);
    // �������Ȳ����� \0
    ds_set_len(s, ds_len(s) + len);

    return s;
}


/* ------------------------------------------------------------
   dynamic string add strings
   ���ܣ���̬�ַ�����׷�Ӷ���ַ���
   ��������̬�ַ��������ĵ�ַ��Ҫд����ַ����������ַ����б�
   ���أ���̬�ַ�������
------------------------------------------------------------ */
char * ds_add_strs(char **ps, int count, ...)
{
    va_list valist;

    va_start(valist, count);

    for (int i=0; i<count; i++) {
        const char *sub = va_arg(valist, char *);
        ds_add_str(ps, sub);
    }

    va_end(valist);

    return *ps;
}


/* ------------------------------------------------------------
   dynamic string insert numbered string
   ���ܣ��ڶ�̬�ַ�����ָ��λ�ò���ָ�����ȵ��Ӵ�
   ����������㡢Ҫ������Ӵ����Ӵ����ȣ�0 ��ʾȫ����
   ���أ���̬�ַ�������
------------------------------------------------------------ */
char * ds_ins_str(char **ps, size_t pos, const char *sub, size_t len)
{
    char *s = *ps;

    if (pos > ds_len(s))
		pos = ds_len(s);

    if (len == 0)
		len = strlen(sub);

    ds_grow(ps, len);
    s = *ps;

    // �ڳ��ռ�
    memcpy(s + pos + len, s + pos, ds_len(s) - pos);
    // �����Ӵ�
    memcpy(s + pos, sub, len);
    // �������Ȳ����� \0
    ds_set_len(s, ds_len(s) + len);

    return s;
}
