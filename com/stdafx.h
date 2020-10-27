#pragma once
#ifndef OS_WIN
#ifndef OS_LINUX
#define OS_LINUX           //在Linux平台下编译
#endif
#ifndef OS_LINUX
#define OS_WIN		      //在WIN平台下编译
#endif
#endif
   
#include "../com/swgap_comm.h"
#ifdef  OS_WIN 
#include "targetver.h" 
#include <direct.h>  
#include <io.h> 
//#include <windows.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
//#ifndef _AFX_NO_AFXCMN_SUPPORT
//#include <afxcmn.h>			// MFC support for Windows Common C
#include "resource.h"    
#include <winsock2.h>
#include <atlstr.h> 
#include <Ws2tcpip.h> 
#include <process.h>
#include <direct.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")

extern "C"
{
//#include "openssl/applink.c"
}; 
#endif
 

#include <sys/stat.h>
#include <errno.h> 
#include <sys/types.h>

#define KBYTE (1024)
#define MBYTE (1024 * KBYTE)
#define GBYTE (1024 * MBYTE)

#define BYTE2KB(n) ((n)/KBYTE)

#define BYTE2MB(n) ((n)/(MBYTE))
#define KBYTE2MB(n) ((n)/(KBYTE))

#define BYTE2GB(n) ((n)/(GBYTE))
#define KBYTE2GB(n) ((n)/(MBYTE))
#define MBYTE2GB(n) ((n)/(KBYTE))
   

//将Win目录路径转化为Linux目录路径
char *ConvertWinToLinuxPath(char *strPathName, int nLen);

//将linux目录路径转化为win目录路径
char *ConvertLinuxToWinPath(char *strPathName, int nLen);

//文件是否是UFT8编码
bool IsTextUTF8(char* str, int length);

//GBK编码转换为UFT8编码
char *GBKToUTF8(char *strGBK);

//UFT8编码转换为GBK编码
char* UTF8ToGBK(char* strUTF8);

//UFT8编码转换为GBK编码
int UTF8ToGBK(char* strUTF8, char *strOutGbk);

//检查目录的合理性
int CheckDir(char *fullpath, char* truefullpath);

//创建目录（根据文件的路径，层层创建）
int create_multi_dir(const char *path);

//更改文件名
int FileRename(const char *srcFile, char* dstFile, int ndstfileLen);

//格式化WIN目录路径
char *FormatWinPath(char *strPathName, int nLen);

//格式化LINUX目录路径
char *FormatLinuxPath(char *strPathName, int nLen);

#ifdef OS_WIN
//删除指定WIN目录（包括目录下的所有文件及子目录）
bool RecursiveDeleteFileForWin( CString path, bool bDelTopDir, bool bDelWritingFile);
#endif

#ifdef OS_LINUX
//删除指定LINUX目录（包括目录下的所有文件及子目录）
bool RecursiveDeleteFileForLinux(char *path, bool bDelTopDir, bool bDelWritingFile);

//Linux平台下UTF8编码转换为GBK编码
int Utf8ToGbkForLinux(char *inbuf, int inlen, char *outbuf, int outlen);

#endif
//Linux平台下GBK编码转换为UTF8编码
int GbkToUtf8ForLinux(char *inbuf, size_t inlen, char *outbuf, size_t outlen);

//检查目录的有效性
int CheckPathValid(char *szPath);

//查找目录下的最老文件
int FindOldestDirPath(char *strSrcPath, char *strOldestPath, int &nOldestTime);
