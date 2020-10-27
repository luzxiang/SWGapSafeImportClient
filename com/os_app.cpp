#include "os_app.h"

//如果文件存在则删除该文件 
int os_remove(char *fname)
{
#ifdef OS_LINUX
        return remove(fname); //如果文件存在则删除该文件 
#endif
#ifdef OS_WIN
        //为了保险起见去掉只读等属性，方能确何删除成功
        SetFileAttributes(fname,FILE_ATTRIBUTE_NORMAL);
        return DeleteFile(fname);//如果文件存在则删除该文件
#endif
}

//路径拼接函数
char *os_pathcat(char *dstpath, const char*srcpath)
{
	if(strlen(dstpath) > 0)
	{
#ifdef OS_WIN
	const char *pchar = "\\";
#else
	const char *pchar = "/";
#endif
	strncat(dstpath , pchar , strlen(pchar));
	}
	strncat(dstpath , srcpath , strlen(dstpath));
	return dstpath;
}

/***************************************************
//通过分隔符格式化目录(前面后面都含有路径分隔符)
	/file/to/dir/
****************************************************/
char* formatDir(char *_dir, int size, const char*dlmto, const char *dlmfrom)
{
	if(_dir == NULL)return _dir;
	char *dir = _dir;
	string todir;

	char *tok=strtok(dir, dlmfrom);
	if(tok == NULL) return _dir;
	do{
		todir += dlmto;
		todir += tok;
	}while((tok=strtok(NULL, dlmfrom))!=NULL);
	OS_SNPRINTF(_dir, size, "%s%s", todir.c_str(),dlmto);
	return _dir;
}
/***************************************************
//通过分隔符格式化路径(前面后面都没有路径分隔符)
	file/to/path.txt
****************************************************/
char* formatPath(char *_path, int size, const char*dlmto, const char *dlmfrom)
{
	if(_path == NULL)return _path;
	char *path = _path;
	string todir;

	char *tok=strtok(path, dlmfrom);
	if(tok == NULL) return _path;
	todir += tok;
	while((tok=strtok(NULL, dlmfrom))!=NULL){
		todir += dlmto;
		todir += tok;
	};
	OS_SNPRINTF(_path, size, "%s", todir.c_str());
	return _path;
}
/***************************************************
//通过分隔符格式化相对路径(前面有路径分隔符，后面没有路径分隔符)
	/file/to/path.txt
****************************************************/
char* formatAbPath(char *_path, int size, const char*dlmto, const char *dlmfrom)
{
	if(_path == NULL)return _path;
	char *path = _path;
	string todir;

	char *tok=strtok(path, dlmfrom);
	if(tok == NULL) return _path;
	do{
		todir += dlmto;
		todir += tok;
	}while((tok=strtok(NULL, dlmfrom))!=NULL);
	OS_SNPRINTF(_path, size, "%s", todir.c_str());
	return _path;
}
/***************************************************
//将路径变为Win平台的相对路径 
//确保不会有多余的"\"路径符号 
	to\win\path
****************************************************/
char* toWinPath(char *path, int size)
{
	char *p = path;
	if(isalpha(*(path)) && *(path+1) == ':')
	{
		if(*(path+2) != '\\')
		{
			perror("Invalid path\n");
			return NULL;
		}
		p = path + 2;
		size -= 2;
	}
	formatPath(p, size, "\\","/");
	formatPath(p, size, "\\","\\");//
	return path;
}
/***************************************************
//将路径变为Win平台的绝对路径 
//确保不会有多余的"\\"路径符号
	D:\to\win\path   \to\win\path
****************************************************/
char* toWinAbPath(char *path, int size)
{
	char *p = path;
	if(isalpha(*(path)) && *(path+1) == ':')
	{
		if(*(path+2) != '\\')
		{
			perror("Invalid path\n");
			return NULL;
		}
		p = path + 2;
		size -= 2;
	}
	formatAbPath(p, size, "\\","/");
	formatAbPath(p, size, "\\","\\");//
	return path;
}
/***************************************************
//将路径变为Linux平台的相对路径 
//确保不会有多余的"/"路径符号
	to/lin/path
****************************************************/
char* toLinPath(char *_path, int size)
{
	char * path = _path;
	(*(path) != '.') ?(_path)
		:((*(path+1) == '/') ? (path = _path + 1)
			: ((*(path+1) == '.' && *(path+2) == '/') ? (path = _path +  2)
				:(_path)))
	;
	formatPath(path, size, "/","\\");
	formatPath(path, size, "/","/");
	return _path;
}
/***************************************************
//将路径变为Linux平台的绝对路径 
//确保不会有多余的"/"路径符号
	/to/lin/path
****************************************************/
char* toLinAbPath(char *_path, int size)
{
	char * path = _path;
	(*(path) != '.') ?(_path)
		:((*(path+1) == '/') ? (path = _path + 1)
			: ((*(path+1) == '.' && *(path+2) == '/') ? (path = _path +  2)
				:(_path)))
	;
	formatAbPath(path, size, "/","\\");
	formatAbPath(path, size, "/","/");
	return _path;
}
/***************************************************
//将路径变为Win平台的目录 
//确保不会有多余的"\"路径符号
	D:\to\win\dir\   \to\win\dir
****************************************************/
char* toWinDir(char *dir, int size)
{
	char *p = dir;
	if(isalpha(*(dir)) && *(dir+1) == ':')
	{
		if(*(dir+2) != '\\')
		{
			perror("Invalid path\n");
			return NULL;
		}
		p = dir + 2;
		size -= 2;
	}
	formatDir(p, size, "\\","/");
	formatDir(p, size, "\\","\\");//
	return dir;
}
/***************************************************
//将路径变为Linux平台的目录 
//确保不会有多余的"/"路径符号
	/to/win/dir/
****************************************************/
char* toLinDir(char *_dir, int size)
{
	char * dir = _dir;
	(*(dir) != '.') ?(_dir)
		:((*(dir+1) == '/') ? (dir = _dir + 1)
			: ((*(dir+1) == '.' && *(dir+2) == '/') ? (dir = _dir +  2)
				:(_dir)))
	;
	formatDir(dir, size, "/","\\");
	formatDir(dir, size, "/","/");
	return _dir;
}
/***************************************************
//将路径变为当前平台的目录
//确保不会有多余的路径符号 
例如： /file/to/dir/
****************************************************/
char* os_format2dir(char *dir, int size)
{
#ifdef OS_WIN
	return toWinDir(dir, size);
#else
	return toLinDir(dir, size);
#endif
	return 0;
}
/***************************************************
//将路径变为当前平台的相对路径
//确保不会有多余的路径符号
//确保不会以路径符号开头和结尾
例如： file/to/dir.txt ;
****************************************************/
char* os_format2path(char *path, int size)
{
#ifdef OS_WIN
	return toWinPath(path, size);
#else
	return toLinPath(path, size);
#endif
	return 0;
}
/***************************************************
//将路径变为当前平台的绝对路径
//确保不会有多余的路径符号
例如： /file/to/dir.txt
****************************************************/
char* os_format2abpath(char *path, int size)
{
#ifdef OS_WIN
	return toWinAbPath(path, size);
#else
	return toLinAbPath(path, size);
#endif
	return 0;
}
//判断文件是否存在
int os_access_fok(const char *filename, int how = 0)
{
#ifdef OS_WIN
	return access(filename, 0);
#else
	return access(filename, F_OK);
#endif
}

/*******************************************************************************
 * Function     : GetDiskFreeSpace
 * Description  : TODO
 * Input        :
 * Return       : 以M为单位
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
unsigned long int GetDiskFreeSpace(const char *path)
{
	int nFreeSize = 0;//以M为单位
#ifdef OS_WIN
		long long i64FreeBytesToCaller = 0;
		long long i64TotalBytes = 0;
		long long i64FreeBytes = 0;			//_int64
		BOOL bRet = false;
		bRet = GetDiskFreeSpaceEx(path,(PULARGE_INTEGER)&i64FreeBytesToCaller,(PULARGE_INTEGER)&i64TotalBytes,(PULARGE_INTEGER)&i64FreeBytes);
		if(!bRet)
		{
			nFreeSize = -1;
		}
		nFreeSize = i64FreeBytesToCaller >> 20;//以M为单位
#endif
#ifdef OS_LINUX
		struct statfs diskInfo = { 0 };
		statfs(path, &diskInfo);

		unsigned long long freeDisk = diskInfo.f_bfree * diskInfo.f_bsize; //剩余空间的大小
		 nFreeSize = freeDisk >> 20;        //以M为单位
#endif
		return nFreeSize;
}












