#ifndef __OS_APP_H__
#define __OS_APP_H__
#include "stdafx.h"
#ifndef OS_WIN 
#ifndef OS_LINUX
#define OS_LINUX
#endif
#ifndef OS_LINUX
#define OS_WIN
#endif
#endif
#ifdef OS_LINUX
#include <sys/types.h>
#include <sys/statfs.h>
#include <dirent.h>
#include <unistd.h>
#include <iconv.h>
#endif  
#ifdef OS_WIN
#include <direct.h>  
#include <io.h> 
#endif 

#include <iostream>   
#include <fstream>  

#include <ctime> 
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <string>
#include <iostream>
#include <sys/stat.h>
using namespace std;

//跨平台函数 
//fopen64
#ifdef OS_WIN
#define OS_SNPRINTF _snprintf
#else 
#define OS_SNPRINTF snprintf
#endif
//snprintf
#ifdef OS_WIN
#define OS_FOPEN fopen
#else 
#define OS_FOPEN fopen64
#endif

//如果文件存在则删除该文件 
extern int os_remove(char *fname);
extern char* os_format2dir(char *dir, int size);
extern char* os_format2path(char *path, int size);
extern char* os_format2abpath(char *path, int size);
//路径拼接函数
extern char *os_pathcat(char *dstpath, const char*srcpath); 
extern int os_access_fok(const char *filename, int); 
extern char* toLinuxPath(char *path);
extern char* toWinPath(char *path);
extern unsigned long int GetDiskFreeSpace(const char *path);
#endif
