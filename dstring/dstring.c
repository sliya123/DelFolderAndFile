#include <stdio.h>   // perror()
#include <stdlib.h>  // malloc()  realloc()  exit()
#include <stdarg.h>  // va_start()  va_arg()  va_end()
#include <string.h>  // memcpy()  strlen()

#include "dstring.h"

/* ============================================================
功能：实现动态字符串（缓冲区容量根据需要自动扩展）
	  动态字符串可以当作普通字符串使用（和普通串特征相同）

内存布局（内存占用 = 32 + 缓冲区长度 + 1）：

 0 -  7：动态字符串标记 "\xDA\xDA""DSTR""\xDC\xDC"（魔法数字）
 8 - 15：自定义，可用于存放编码类型等
24 - 31：字符串长度（字节）
16 - 23：缓冲区长度（字节）
32 -  n：缓冲区空间
n + 1  ：'\0' 字符，保证缓冲区以 '\0' 结尾
============================================================ */

// 动态字符串结构体
typedef struct {
	uint64_t magic;   // 魔法数字
	uint64_t custom;  // 自定义数据
	uint64_t len;     // 字符串长度（不包括最后的 \0）
	uint64_t cap;     // 缓冲区长度（不包括最后的 \0）
	char data[];      // 缓冲区（始终以 \0 结尾）
} dstr;

static const int DS_HEADER_SIZE = sizeof(uint64_t) * 4;   // 头部大小
static const uint64_t MAGIC_NUMBER = 0xDCDC52545344DADA;  // 魔法数字

// 从数据区（data）到各个字段的 uint64_t 偏移量
static const int OFF_MAGIC	 = -4;
static const int OFF_CUSTOM	 = -3;
static const int OFF_LEN     = -2;
static const int OFF_CAP     = -1;


/* ------------------------------------------------------------
   dynamic string get header
   功能：从动态字符串的数据位置跳转到头部位置
   参数：动态字符串变量
   返回：动态字符串结构体指针
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
   功能：创建动态字符串
   参数：要预留的缓冲区大小
   返回：动态字符串变量（即：缓冲区中数据的地址）
------------------------------------------------------------ */
char * new_ds(size_t cap)
{
	dstr *ds;

	// 预留尾部的 '\0' 字节
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
	// 因为申请内存时 +1 个字节，所以当参数 cap 为 0 时才能写入这个字节
	ds->data[0] = '\0';

	return ds->data;
}


/* ------------------------------------------------------------
   free dynamic string
   功能：释放动态字符串，并将 *ps 设置为 NULL
   参数：动态字符串变量的地址
   返回：无
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
   功能：判断字符串是否为动态字符串
   参数：字符串变量
   返回：是返回 1，不是返回 0
------------------------------------------------------------ */
int is_ds(const char *s)
{
	return ((uint64_t *)s)[OFF_MAGIC] == MAGIC_NUMBER;
}


/* ------------------------------------------------------------
   dynamic string custom data
   功能：获取动态字符串的自定义数据
   参数：动态字符串变量
   返回：自定义数据
------------------------------------------------------------ */
uint64_t ds_custom(const char *s)
{
	return ((uint64_t *)s)[OFF_CUSTOM];
}


/* ------------------------------------------------------------
   dynamic string custom data
   功能：设置动态字符串的自定义数据
   参数：动态字符串变量，定义数据
   返回：无
------------------------------------------------------------ */
void ds_set_custom(char *s, uint64_t custom)
{
	((uint64_t *)s)[OFF_CUSTOM] = custom;
}


/* ------------------------------------------------------------
   dynamic string length
   功能：获取动态字符串的长度
   参数：动态字符串变量
   返回：字符串长度（不包括尾部的 '\0'）
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
   功能：获取动态字符串的缓冲区容量
   参数：动态字符串变量
   返回：缓冲区容量（不包括尾部的 '\0'）
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
   功能：获取动态字符串的最后一个字符
   参数：动态字符串变量
   返回：最后一个字符，如果字符串为空，则返回 '\0'
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
   功能：获取动态字符串最后一个字符之后的位置
   参数：动态字符串变量
   返回：位置指针
------------------------------------------------------------ */
char * ds_end(char *s)
{
	return s + ds_len(s);
}


/* ------------------------------------------------------------
   grow dynamic string
   功能：根据 needsize 判断是否需要扩容，根据需要执行扩容
   参数：动态字符串变量的地址，要申请的可用空间大小
   返回：动态字符串变量
------------------------------------------------------------ */
// 使用 Fast Inverse Square Root 算法快速求平方根
float fast_sqrt(float x)
{
	float xhalf = 0.5f * x;
	int i = *(int*)&x;               // 获取浮点数的二进制位
	i = 0x5f375a86 - (i >> 1);       // 给出初始猜测 y0
	x = *(float*)&i;                 // 将二进制位转换回浮点
	x = x * (1.5f - xhalf * x * x);  // 牛顿步骤，重复提高精度
	return 1/x;
}

char * ds_grow(char **ps, size_t needsize)
{
	char *s = *ps;

	uint64_t len = ds_len(s) + needsize;
	uint64_t cap = ds_cap(s);

	// 空间足够，无需扩容
	if (len <= cap)
		return s;

	// 根据数据大小，决定预留多少空间
	if (len > 128*1024*1024)
	    // len 大于 128M，则预留 32M
		cap = len + 32*1024*1024;
	else if (len > 4*1024*1024)
		// len 在 (4M,128M] 之间，则根据平方根函数预留
		// 从预留 len*2 逐渐过渡到预留 len/4
		// 从 2M 增长到 128M 会增长 8 次
		// len 超过 255M，则 FastSqrt 可能溢出
		cap = len + fast_sqrt(len * 8) * 1024;
	else if (len < 16)
		// 最少 16 字节
		cap = 16;
	else
		// 小于 4M，翻倍预留
		cap = len * 2;

	return ds_resize(ps, cap);
}


/* ------------------------------------------------------------
   resize dynamic string capacity
   功能：调整动态字符串的容量（会自动在容量后面添加 '\0'）
   参数：动态字符串变量的地址，新的容量
   返回：动态字符串变量
------------------------------------------------------------ */
char * ds_resize(char **ps, size_t cap)
{
	char *s = *ps;

	// 包含尾部的 '\0' 位置
	s = realloc(s - DS_HEADER_SIZE, DS_HEADER_SIZE + cap + 1);
	if (s == NULL)
	{
		perror("resizeds()->realloc()");
		exit(1);
	}
	// 指向数据区
	s += DS_HEADER_SIZE;

	// 如果调整后的容量小于原来的字符串长度，则修正
	if (ds_len(s) > cap)
		// 修正长度并补上 \0
		ds_set_len(s, cap);

    // 修正容量并补上 \0
	ds_set_cap(s, cap);

	*ps = s;

	return s;
}


/* ------------------------------------------------------------
   pack dynamic string
   功能：压缩动态字符串（释放预留空间，只保留数据部分）
   参数：动态字符串变量的地址
   返回：动态字符串变量
------------------------------------------------------------ */
char * ds_pack(char **ps)
{
	return ds_resize(ps, ds_len(*ps));
}


/* ------------------------------------------------------------
   dynamic string add char
   功能：向动态字符串中追加字符
   参数：动态字符串变量的地址，要写入的字符
   返回：动态字符串变量
------------------------------------------------------------ */
char * ds_add_char(char **ps, char c)
{
    ds_grow(ps, 1);

    char *s = *ps;

    ds_end(s)[0] = c;
    // 修正长度并补上 \0
    ds_set_len(s, ds_len(s) + 1);

    return s;
}

/* ------------------------------------------------------------
   dynamic string add string
   功能：向动态字符串中追加字符串
   参数：动态字符串变量的地址，要写入的字符串
   返回：动态字符串变量
------------------------------------------------------------ */
char * ds_add_str(char **ps, const char* sub)
{
	size_t len = strlen(sub);
    ds_grow(ps, len);

    char *s = *ps;

    memcpy(ds_end(s), sub, len);
    // 修正长度并补上 \0
    ds_set_len(s, ds_len(s) + len);

    return s;
}


/* ------------------------------------------------------------
   dynamic string add strings
   功能：向动态字符串中追加多个字符串
   参数：动态字符串变量的地址，要写入的字符串个数，字符串列表
   返回：动态字符串变量
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
   功能：在动态字符串的指定位置插入指定长度的子串
   参数：插入点、要插入的子串、子串长度（0 表示全部）
   返回：动态字符串变量
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

    // 腾出空间
    memcpy(s + pos + len, s + pos, ds_len(s) - pos);
    // 插入子串
    memcpy(s + pos, sub, len);
    // 修正长度并补上 \0
    ds_set_len(s, ds_len(s) + len);

    return s;
}
