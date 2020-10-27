#include "stdafx.h"
#include <iostream>  
#include <string>  
#include <fstream> 
#include <string.h>
#include <string>
#include <ctime>
#include "../log/log.h"
#ifdef OS_LINUX
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <iconv.h>
#include <sys/stat.h>
#endif

using namespace std;
 

/****************************************************************************************
 Function:       SLEEP
 Description:    通用的Sleep函数 输入毫秒数
 Calls:
 Table Accessed:
 Table Updated:
 Input:          nSleep,睡眠毫秒数
 Output:
 Return:
 Others:
 ****************************************************************************************/
//void SLEEP(int slp_ms)
//{
//#ifdef OS_LINUX
//    usleep(slp_ms * 1000);
//#else
//    Sleep(slp_ms);
//#endif
//}
//void SLEEP(uint slp_ms, int slp_us)
//{
//#ifdef OS_LINUX
//    usleep(slp_ms * 1000 + slp_us);
//#else
//    slp_ms += 0*slp_us;
//    Sleep(slp_ms);
//#endif
//}
/****************************************************************************************
 Function:       FormatLinuxPath
 Description:    格式化Linux路径（去掉多余的"/"）
 Calls:
 Table Accessed:
 Table Updated:
 Input:          strPathName, 要格式化的目录文件名
 nLen,  目录文件名的内存空间大小
 Output:         strPathName, 格式化后输出的目录文件名
 Return:         NULL, 目录文件名为空, 非空时为格式化输出的目录文件名
 Others:
 ****************************************************************************************/
char *FormatLinuxPath(char *strPathName, int nLen)
{
    if (NULL == strPathName || nLen <= 0)
        return NULL;
    int i = 0, j = 0;
    for (i = 0; i < nLen; i++)
    {
        if ((i + 1) < nLen)
        {
            if (strPathName[i] == '/' && strPathName[i + 1] == '/')
            {
                j++;
                /*在Debian 4.8.4-1操作系统下有问题(在其它操作系统没问题)，如将
                 /usr/safetemp/1000//bcp/device_info.bcp.1487295257 转为
                 /usr/safetemp/1000/bcp/device_inf..bcp.1487295257，因此改为单个字节赋值
                 memcpy(&strPathName[i+1], &strPathName[i+2], nLen-(i+2)); */
                for (int k = 0; k < nLen - (i + 2); k++)
                {
                    if ((i + 2 + k) < nLen)
                    {
                        strPathName[i + 1 + k] = strPathName[i + 2 + k];
                    }
                    else
                    {
                        strPathName[i + 1 + k] = '\0';
                    }
                }
                strPathName[nLen - j] = 0;
                i = 0;      //从０开始扫
            }
        }
    }
    return strPathName;
}

/****************************************************************************************
 Function:       FormatWinPath
 Description:    格式化WIN路径（去掉多余的"\"）
 Calls:
 Table Accessed:
 Table Updated:
 Input:          strPathName, 要格式化的目录文件名
 nLen,  目录文件名的内存空间大小
 Output:         strPathName, 格式化后输出的目录文件名
 Return:         NULL, 目录文件名为空, 非空时为格式化输出的目录文件名
 Others:
 ****************************************************************************************/
char *FormatWinPath(char *strPathName, int nLen)
{
    if (NULL == strPathName || nLen <= 0)
        return NULL;
    int i = 0, j = 0;
    for (i = 0; i < nLen; i++)
    {
        if ((i + 1) < nLen)
        {
            if (strPathName[i] == '\\' && strPathName[i + 1] == '\\')
            {
                j++;
                if ((i + 2) < nLen)
                {
                    memcpy(&strPathName[i + 1], &strPathName[i + 2], nLen - (i + 2));
                }
                else
                {
                    strPathName[i + 1] = '\0';
                }
                strPathName[nLen - j] = 0;
                i = 0;      //从０开始扫
            }
        }
    }
    return strPathName;
}

/****************************************************************************************
 Function:       ConvertWinToLinuxPath
 Description:    将WIN目录格式转化为Linux目录格式
 Calls:
 Table Accessed:
 Table Updated:
 Input:          strPathName, 要转化的目录文件名
 nLen,  目录文件名的内存空间大小
 Output:         strPathName, 转化后输出的目录文件名
 Return:         NULL, 目录文件名为空, 非空时为转化输出的目录文件名
 Others:
 ****************************************************************************************/
char *ConvertWinToLinuxPath(char *strPathName, int nLen)
{
    if (NULL == strPathName || nLen <= 0)
        return NULL;
    for (int i = 0; i < nLen; i++)
    {
        if (strPathName[i] == '\\')
            strPathName[i] = '/';
    }
    return strPathName;
}

/****************************************************************************************
 Function:       ConvertLinuxToWinPath
 Description:    将Linux目录格式转化为Win目录格式
 Calls:
 Table Accessed:
 Table Updated:
 Input:          strPathName, 要转化的目录文件名
 nLen,  目录文件名的内存空间大小
 Output:         strPathName, 转化后输出的目录文件名
 Return:         NULL, 目录文件名为空, 非空时为转化输出的目录文件名
 Others:
 ****************************************************************************************/
char *ConvertLinuxToWinPath(char *strPathName, int nLen)
{
    if (NULL == strPathName || nLen <= 0)
        return NULL;
    for (int i = 0; i < nLen; i++)
    {
        if (strPathName[i] == '/')
            strPathName[i] = '\\';
    }
    return strPathName;
}

/****************************************************************************************
 Function:       IsTextUTF8
 Description:    判断字符串是否是UTF-8编码
 Calls:
 Table Accessed:
 Table Updated:
 Input:          strUft8, 要判断的字符串
 nLen,  strUft8的内存空间大小
 Output:
 Return:         false, 该字符串不是UTF8字符串
 true, 该字符串是UTF8字符串
 Others:
 ****************************************************************************************/
bool IsTextUTF8(char* strUft8, int nLen)
{
    if (NULL == strUft8 || nLen)
        return false;
    int nBytes = 0;      //UFT8可用1-6个字节编码,ASCII用一个字节
    unsigned char chr;
    bool bAllAscii = true; //如果全部都是ASCII, 说明不是UTF-8
    for (int i = 0; i < nLen; ++i)
    {
        chr = *(strUft8 + i);
        if ((chr & 0x80) != 0) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
            bAllAscii = false;
        if (nBytes == 0) //如果不是ASCII码,应该是多字节符,计算字节数
        {
            if (chr >= 0x80)
            {
                if (chr >= 0xFC && chr <= 0xFD)
                    nBytes = 6;
                else if (chr >= 0xF8)
                    nBytes = 5;
                else if (chr >= 0xF0)
                    nBytes = 4;
                else if (chr >= 0xE0)
                    nBytes = 3;
                else if (chr >= 0xC0)
                    nBytes = 2;
                else
                    return false;
                nBytes--;
            }
        }
        else //多字节符的非首字节,应为 10xxxxxx
        {
            if ((chr & 0xC0) != 0x80)
                return false;
            nBytes--;
        }
    }
    if (nBytes > 0) //违返规则
        return false;
    if (bAllAscii) //如果全部都是ASCII, 说明不是UTF-8
        return false;
    return true;
}

/****************************************************************************************
 Function:       GBKToUTF8
 Description:    GBK编码转为UTF8编码
 Calls:
 Table Accessed:
 Table Updated:
 Input:          strGBK, 要转的字符串
 Output:
 Return:         NULL, 该字符串为空
 非空, 转换后的UTF8字符串指针（注意：在调用GBKToUTF8后，一定要释放该内存指针）
 Others:
 ****************************************************************************************/
char *GBKToUTF8(char *strGBK)
{
    if (NULL == strGBK)
        return NULL;
    char* strOutUTF8 = NULL;
#ifdef OS_WIN
    WCHAR * str1;
    int n = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
    str1 = new WCHAR[n];
    MultiByteToWideChar(CP_ACP, 0, strGBK, -1, str1, n);
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
    char * str2 = new char[n];
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
    strOutUTF8 = str2;
    delete[]str1;
    str1 = NULL;
#endif
    return strOutUTF8;
}

/****************************************************************************************
 Function:       UTF8ToGBK
 Description:    UTF8编码转为GBK编码
 Calls:
 Table Accessed:
 Table Updated:
 Input:          strUTF8, 要转的字符串
 Output:
 Return:         NULL, 该字符串为空
 非空, 转换后的UTF8字符串指针（注意：在调用GBKToUTF8后，一定要释放该内存指针）
 Others:
 ****************************************************************************************/
char *UTF8ToGBK(char* strUTF8)
{
    if (NULL == strUTF8)
        return NULL;
    char *strOutGbk = NULL;
#ifdef OS_WIN
    int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
    if(len < 1)
    return NULL;
    WCHAR * wszGBK = new WCHAR[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    if(MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len) < 1)
    {
        delete []wszGBK;
        return NULL;
    }
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    if(len < 1)
    {
        delete []wszGBK;
        return NULL;
    }
    char *szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    if(WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL) < 1)
    {
        delete []wszGBK;
        delete []szGBK;
        return NULL;
    }
    strOutGbk = szGBK;
    delete[]wszGBK;
    return strOutGbk;
#endif
    return strOutGbk;

}

/****************************************************************************************
 Function:       create_multi_dir
 Description:    根据文件名创建目录
 Calls:
 Table Accessed:
 Table Updated:
 Input:          filename, 要创建的的目录文件名
 Output:
 Return:         -1, 目录文件名为空，或创建目录失败
 >=0 创建成功
 Others:
 ****************************************************************************************/
int create_multi_dir( const char *filename )
{
	if ( NULL == filename )
	{
		return -1;
	}

	int i = 0 , len = 0;
	len = strlen( filename );
	char *dir_path = ( char* ) malloc( len + 1 );
//	LOG_DEBUG("error=%d,%s\n",errno,strerror(errno));
	dir_path[len] = '\0';
	strncpy( dir_path , filename , len );
#ifdef OS_LINUX
	for ( i = 0; i < len ; i++ )
	{
		//            ch = dir_path[i];
		if ( dir_path[i] == '/' && i > 0 )
		{
			dir_path[i] = '\0';       //以此为字符串结束
//			LOG_WATCH(dir_path);
			if ( 0 != access( dir_path , F_OK ) )
			{
				if ( 0 != mkdir( dir_path , 0755 ) )
				{
					if ( 0 != access( dir_path , F_OK ) )
					{
						printf( "mkdir=%s\n" , dir_path );
						free( dir_path );
						return -1;
					}
				}
			}
			dir_path[i] = '/';        //重新连上
		}
	}
#else
//	LOG_WATCH(dir_path);
	for (i=0; i<len; i++)
	{
		//            ch = dir_path[i];
		if (dir_path[i] == '\\' && i > 0)
		{
			dir_path[i]='\0';           //以此为字符串结束
//	LOG_WATCH(dir_path);
			if (access(dir_path, 0) < 0)
			{
//				LOG_DEBUG("mkdir(dir_path):%s",dir_path);
				if (mkdir(dir_path) < 0)
				{
					free(dir_path);
					return -1;
				}
				/*//if (MKDIR(dir_path, 0755) < 0)
				 WCHAR wszClassName[256];
				 memset(wszClassName,0,sizeof(wszClassName));
				 MultiByteToWideChar(CP_ACP,0,dir_path, strlen(dir_path)+1, wszClassName, sizeof(wszClassName)/sizeof(wszClassName[0]));
				 if(CreateDirectory(wszClassName, NULL) != 1)
				 {
				 PRINT_MSG("mkdir=%s\n", dir_path);
				 free(dir_path);
				 return -1;
				 }*/
			}
			dir_path[i]='\\';           //重新连上
		}
	}
#endif
	free( dir_path );
	return 0;
}

#ifdef OS_WIN
/****************************************************************************************
 Function:       RecursiveDeleteFileForWin
 Description:    删除Window下指定的目录及文件
 Calls:
 Table Accessed:
 Table Updated:
 Input:          szpath, 要删除的目录文件名
 bDelTopDir，是否删除顶层目录，(true,删除顶层目录 false, 不删除顶层目录)
 bDelWritingFile, 是否删除正在写操作的文件(true,删除 false, 不删除)
 Output:
 Return:         false, 删除失败
 true, 删除成功
 Others:
 ****************************************************************************************/
bool RecursiveDeleteFileForWin( CString szPath, bool bDelTopDir, bool bDelWritingFile)
{
    CFileFind ff;
    CString path = szPath;
    if(path.Right(1) != "\\")
    path += "\\";
    path += "*.*";
    BOOL res = ff.FindFile(path);
    while(res)
    {
        res = ff.FindNextFile();
        //是文件时直接删除
        if (!ff.IsDots() && !ff.IsDirectory())
        {
            if(bDelWritingFile)
            {
                SetFileAttributes(ff.GetFilePath(),FILE_ATTRIBUTE_NORMAL);
                DeleteFile(ff.GetFilePath());
            }
            else
            {
                char strFileName[1024];
                sprintf(strFileName, "%s", (LPCTSTR)ff.GetFilePath());
                //SendFileData(strFileName);
                time_t tNow = time(NULL);
                struct _stat64 file_stat;//支持大文件 大于3G的
                _stat64(strFileName,&file_stat);
                if((tNow - file_stat.st_mtime) >3)//最后修改时间
                {
                    SetFileAttributes(ff.GetFilePath(),FILE_ATTRIBUTE_NORMAL);
                    DeleteFile(ff.GetFilePath());	//已经写完毕,文件可以删除
                }
            }
        }
        else if (ff.IsDots())
        continue;
        else if (ff.IsDirectory())
        {
            path = ff.GetFilePath();
            //是目录时继续递归，删除该目录下的文件
            RecursiveDeleteFileForWin(path, bDelTopDir, bDelWritingFile);
            //目录为空后删除目录
            RemoveDirectory(path);
        }
    }
    //最终目录被清空了，于是删除该目录
    if(bDelTopDir)
    RemoveDirectory(szPath);//是否删除最顶层的目录
    return true;
}

#endif

#ifdef OS_LINUX
/****************************************************************************************
 Function:       RecursiveDeleteFileForLinux
 Description:    删除linux下指定的目录及文件
 Calls:
 Table Accessed:
 Table Updated:
 Input:          path, 要删除的目录文件名
 bDelTopDir，是否删除顶层目录，(true,删除顶层目录 false, 不删除顶层目录)
 bDelWritingFile, 是否删除正在写操作的文件(true,删除 false, 不删除)
 Output:
 Return:         false, 删除失败
 true, 删除成功
 Others:
 ****************************************************************************************/
bool RecursiveDeleteFileForLinux(char *path, bool bDelTopDir, bool bDelWritingFile)
{
    if (NULL == path)
        return false;
    DIR *dir = NULL;
    struct dirent *s_dir;
    struct stat file_stat;
    char currfile[PATH_LEN + FILE_NAME_LEN] = { 0 };
    char strPath[PATH_LEN] = { 0 };
    snprintf(strPath, (sizeof(strPath)), "%s", path);
    int len = strlen(path);
    if (strPath[len - 1] != '/')
    {
        strPath[len] = '/';
        strPath[len + 1] = 0;
    }
    if ((dir = opendir(strPath)) == NULL)   //PWD List
    {
        LOG_INFO("opendir(strPath) error.");
        return -1;
    }
    while ((s_dir = readdir(dir)) != NULL)
    {
        if ((strcmp(s_dir->d_name, ".") == 0) || (strcmp(s_dir->d_name, "..") == 0))
            continue;
        sprintf(currfile, "%s%s", strPath, s_dir->d_name);
        stat(currfile, &file_stat);
        if (S_ISDIR(file_stat.st_mode))      //目录名
        {
            RecursiveDeleteFileForLinux(currfile, bDelTopDir, bDelWritingFile);
            if (bDelWritingFile)
                remove(currfile);
            else
            {
                time_t tNow = time(NULL);
                if ((tNow - file_stat.st_mtime) > 3) //最后修改时间
                {
                    remove(currfile);       //已经写完毕,文件可以删除
                }
            }
        }
        else
        {
            remove(currfile);
        }
    }
    closedir(dir);
    //最终目录被清空了，于是删除该目录
    if (bDelTopDir)
    {
        //remove(currfile);                   //删除最顶层的目录
        remove(path);
    }
    return true;
}
#endif

/****************************************************************************************
 Function:       CheckDir
 Description:    检查目录的合理性（用于规范文件目录名）
 Calls:
 Table Accessed:
 Table Updated:
 Input:          fullpath, 待检查的源目录文件名
 Output:         truefullpath, 检查后输出的目录目标目录文件名
 Return:         -1, 源文件或目标文件为空
 >=0,改名成功
 Others:
 ****************************************************************************************/
int CheckDir(char *fullpath, char* truefullpath)
{
    if (NULL == fullpath || NULL == truefullpath)
        return -1;
#ifdef OS_LINUX
    DIR *dir;
    int pathlen = 0, i = 0;

    if ((dir = opendir(fullpath)) == NULL)
    {
        printf("opendir fullpath error=%s\n", fullpath);
        return -1;
    }
    closedir(dir);
    pathlen = strlen(fullpath);
    if (pathlen - 1 != 0)		// 路径长度不为1  的目录
    {
        if (fullpath[pathlen - 1] == '/')	// 最后字节是'/'
        {
            for (i = pathlen - 1; i >= 0; i--)
            {
                if (fullpath[i - 1] != '/' && fullpath[i] == '/')
                    break;
            }
            fullpath[i + 1] = 0;
        }
        else		// 最后字节不是'/'
        {
            fullpath[pathlen] = '/';
            fullpath[pathlen + 1] = 0;
        }
    }
    else 	// 路径长度为1  的目录
    {
        if (fullpath[pathlen - 1] != '/')
        {
            fullpath[pathlen] = '/';
            fullpath[pathlen + 1] = 0;
        }
        else
            fullpath[pathlen] = 0;
    }
    strcpy(truefullpath, fullpath);
#endif
    return 0;
}

/****************************************************************************************
 Function:       CheckPathValid
 Description:    检测目录的有效性(不能是只有盘符而没有具体的目录名；不能为空)， 必须为绝对路径
 Calls:
 Table Accessed:
 Table Updated:
 Input:          szPath, 待检查的源目录文件名
 Output:
 Return:         -1, 源文件或目标文件为空
 -2,-3,-4 目录格式不对
 >=0,目录有效
 Others:
 ****************************************************************************************/
int CheckPathValid(char *strTempPath)
{
    if (NULL == strTempPath)
        return -1; 
#ifdef OS_WIN
    if(strlen(strTempPath) < 4)
		return -1;
	if(!isalpha(strTempPath[0]))
        return -2;
    if(strTempPath[1] != ':' && strTempPath[2] != '\\')
    {
        return -3;
    }
    if(strTempPath[3]== '\0' ) return -4;
#endif

#ifdef OS_LINUX
    if (strlen(strTempPath) < 2) 
        return -1;    
	if ( strTempPath[0] != '/' )
		return -3;
	if ( strTempPath[1] == '\0' )
		return -4;
#endif 
	return 0;
}
#ifdef OS_LINUX

/****************************************************************************************
 Function:       code_convert
 Description:    编码转换
 Calls:
 Table Accessed:
 Table Updated:
 Input:          from_charset, 源编码
 to_charset,   转换目标编码
 inbuf, 源编码字符串指针
 inlen,源编码字符串内存空间大小
 Output:         outbuf,存放目标编码字符串指针
 outlen,目标编码字符串空间大小
 Return:         -1, 源编码或目标编码或字符串缓冲区为空
 >=0,转换成功
 Others:
 ****************************************************************************************/
int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    if (NULL == from_charset || NULL == to_charset || NULL == inbuf || NULL == outbuf)
        return -1;
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;
    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
    {
        return -1;
    }
    int rt = (int) iconv(cd, pin, &inlen, pout, &outlen);
    if (rt == -1)
    {
        iconv_close(cd);
        return -1;
    }
    iconv_close(cd);
    return 0;
}

/****************************************************************************************
 Function:       Utf8ToGbkForLinux
 Description:    Linux平台下Utf8编码转换为GBK编码
 Calls:
 Table Accessed:
 Table Updated:
 Input:          inbuf, 源编码字符串指针
 inlen,源编码字符串内存空间大小
 Output:         outbuf,存放目标编码字符串指针
 outlen,目标编码字符串空间大小
 Return:         -1, 源编码或目标编码或字符串缓冲区为空
 >=0,转换成功
 Others:
 ****************************************************************************************/
int Utf8ToGbkForLinux(char *inbuf, int inlen, char *outbuf, int outlen)
{
    if (NULL == inbuf || NULL == outbuf)
        return -1;
    char CodeSrc[20] = { 0 };
    char CodeDest[20] = { 0 };
    strcpy(CodeSrc, "utf-8");
    strcpy(CodeDest, "gb2312");
    return code_convert(CodeSrc, CodeDest, inbuf, inlen, outbuf, outlen);
}


/****************************************************************************************
 Function:       GbkToUtf8ForLinux
 Description:    Linux平台下GBK编码转换为UTF8编码
 Calls:
 Table Accessed:
 Table Updated:
 Input:          inbuf, 源编码字符串指针
 inlen,源编码字符串内存空间大小
 Output:         outbuf,存放目标编码字符串指针
 outlen,目标编码字符串空间大小
 Return:         -1, 源编码或目标编码或字符串缓冲区为空
 >=0,转换成功
 Others:
 ****************************************************************************************/
int GbkToUtf8ForLinux(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    if (NULL == inbuf || NULL == outbuf)
        return -1;
    char CodeSrc[10] = { 0 };
    char CodeDest[10] = { 0 };
    strcpy(CodeSrc, "utf-8");
    strcpy(CodeDest, "gb2312");
    return code_convert(CodeDest, CodeSrc, inbuf, inlen, outbuf, outlen);
}

#endif
