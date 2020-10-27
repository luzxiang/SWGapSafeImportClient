/*
 * FileTask.cpp
 *
 *  Created on: 2018年9月6日
 *      Author: luzxiang
 */
#include "../com/stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifdef  OS_LINUX
#include <dirent.h>
#include <unistd.h>
#endif
#ifdef OS_WIN
#include <direct.h>  
#include <io.h> 
#endif
#include <fcntl.h>
#include <algorithm>
#include <string.h>
#include <string>
#include <iterator>
#include <chrono>
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>    // std::condition_variable

#include "../log/log.h"
#include "../Main/config.h"
#include "../Analyse/Analyse.h"
#include "../com/swgap_comm.h"
#include "../com/os_app.h"
#include "FileTask.h"

extern Socket *g_Socket;
extern Analyse *g_AnalyseLayer;

/*******************************************************************************
 * Function     : FileTask::FileTask
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
FileTask::FileTask(FILESYNC_RULE &r)
{
	IsEnable = false;
    FileRespState = -1;      	//发送文件响应状态      0,无响应，1有响应(成功) ，2有响应(失败)
    TaskRespState = -1;      	//任务响应状态      0,无响应，1有响应(成功) ，2有响应(失败)
	stableTime = r.nSynTime;
	ScanLimitSize = r.nLimitSpeed;
	nTaskID = r.nTaskID;
	move_fun = rename;
	tLastSync = time(NULL);    	//最后一次同步时间
    memset(&CurFile,0,sizeof(CurFile));
    memset(&CurSend,0,sizeof(CurSend));
    memset(&CurRecv,0,sizeof(CurRecv));
    transMod = GetTransMod((TRANSDIRT)r.nTransmod, Config::SvrInfo.nWorkMode);

	TaskPath.Reset();
	TempPath.Reset();
	SendBuf.Reset();
	memset(&rule,0,sizeof(rule));
	memcpy(&rule,&r,sizeof(rule));
	InitTask();
	LOG_DEBUG("单向任务创建成功");
}
/*******************************************************************************
 * Function     : FileTask::~FileTask
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
FileTask::~FileTask(void){

}
 
/*******************************************************************************
 * Function     : FileTask::NewTask
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
bool FileTask::InitTask(void)
{
	os_format2dir(this->rule.strSrcPath,sizeof(this->rule.strSrcPath));
	memset(this->TaskPath.workPath, 0, sizeof(this->TaskPath.workPath));
	strcpy(this->TaskPath.workPath, this->rule.strSrcPath);
	if (CheckPathValid(this->TaskPath.workPath) < 0)
	{
		LOG_WARN("strSrcPath is avalid! srcpath=%s", this->TaskPath.workPath);
		return false;
	}  
	if (create_multi_dir(this->TaskPath.workPath) < 0)
	{
		LOG_WARN("failure to create_multi_dir:%s", this->TaskPath.workPath);
		return false;
	}
	memset(this->TempPath.workPath, 0, sizeof(this->TempPath.workPath));
	OS_SNPRINTF(this->TempPath.workPath, sizeof(this->TempPath.workPath), "%s/%d/",
			Config::Sender.strTmpPath, this->rule.nTaskID);
	os_format2dir(this->TempPath.workPath,sizeof(this->TempPath.workPath));
	if (CheckPathValid(this->TempPath.workPath) < 0)
	{
		LOG_WARN("strSrcPath is avalid! srcpath=%s", this->TempPath.workPath);
		return false;
	}
	if (create_multi_dir(this->TempPath.workPath) < 0)
	{
		LOG_WARN("failure to create_multi_dir:%s", this->TempPath.workPath);
		return false;
	}  
	LOG_INFO("SrcPath:%s",this->TaskPath.workPath);
	LOG_INFO("TempPath:%s",this->TempPath.workPath);
	LOG_INFO("VirusPath:%s",Config::Sender.strVirusPath);
	LOG_INFO("TaskID=%d, taskName=%s, srcPath=%s, "
			"transmod=%d, limitspeed=%d, nStableTime=%d, "
			"filecheck=%d, status=%d",
			this->rule.nTaskID, this->rule.strTaskName, this->rule.strSrcPath,
			this->rule.nTransmod, this->rule.nLimitSpeed, this->rule.nSyncTime,
			this->rule.nLinkFileCheckId, this->rule.status);
	return true;
}
/*******************************************************************************
 * Function     : FileTask::Release
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --m_bLogin
 *******************************************************************************/
void FileTask::Release(void){
    if (this->CurFile.pHandle)
    {
        fclose(this->CurFile.pHandle);
        this->CurFile.pHandle = NULL;
    }
}

/*******************************************************************************
 * Function     : FileTask::GetTransDirect
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
TRANSMOD FileTask::GetTransMod(TRANSDIRT transmod, int inorout){

	TRANSMOD workmod = SWGAP_SND; 
    switch (transmod ^ inorout)
    {
        case 1:
        case 3:
            workmod = SWGAP_RCV;
            break;
        case 0:
        case 2:
            workmod = SWGAP_SND;
            break;
    }
    return workmod;
}
/*******************************************************************************
 * Function     : FileTask::TaskTempFileSend
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::SendTempFileFromBuf(void){

	int res = 0;
	unsigned int nSessionID = 1;
	for(auto iter : this->SendBuf.List)
	{
		res = this->SendFileData(iter, nSessionID++);
		SLEEP_US(Config::Sender.nIOSleep);
		if (0 > res)//文件不存在
		{
			LOG_WARN("false to send file: %s\n", iter.strFileName);
			if (-1 == res)
				continue;
			return -1;
		}
	}
	return 0;
}
/*******************************************************************************
 * Function     : FileTask::handtmpdir
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
bool FileTask::HandTmpFile(void){

    if (this->rule.status == 0)
    {
        LOG_TRACE("Task:%d status == 0", this->rule.nTaskID);
		return true;
    }
	if (SWGAP_SND != this->transMod)
	{
		LOG_TRACE("Task:%d,SWGAP_SND != GetTransDirect", this->rule.nTaskID);
		return true;
	}
	LOG_DEBUG("begin to start scan the file of temp");
	if(this->TempPath.List.empty())
	{
		this->TempPath.Reset(); 
		this->ScanPathFileTo(this->TempPath.workPath,this->TempPath);
		LOG_DEBUG("在Temp目录中，扫描到%d个文件",this->TempPath.List.size()); 
		if(this->TempPath.List.empty())
		{
			LOG_WATCH(this->TempPath.List.empty());
			this->TempPath.Reset();
		}else{
			this->TempPath.Flag = 2;
		}
	}
	LOG_DEBUG("Finish scan the file of temp");
	return this->TempPath.List.empty();
}
/*******************************************************************************
 * Function     : FileTask::ScanSrcPath
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::ScanPathFileTo(char *srcPath, FileList_St& fList){
	if(srcPath == NULL)
		return -1;
	os_format2dir(srcPath,1024);
#ifdef OS_LINUX
	DIR *dir = NULL;
	struct dirent *s_dir;
	string virType("|");
	virType.append(this->rule.strFileType);
	virType.append("|");
	string suffix;
	bool fix_flag = strlen(this->rule.strFileType) > 0;
	char curdir[PATH_LEN + FILE_NAME_LEN] =
	{ 0 };

	if ((dir = opendir(srcPath)) == NULL)   //PWD List
	{
		if (ENOENT == errno)
		{
			if (create_multi_dir(srcPath) < 0)
			{
				LOG_WARN("failure to create_multi_dir:%s", srcPath);
				return -1;
			}
		}
		else
		{
			LOG_ERROR("opendir(%s) error,errno:%d,error:%s"
					, srcPath, errno, strerror(errno));
			return -1;
		}
	}

	time_t tNow = time( NULL);
	struct stat file_stat;
	int stat_flag = 0;
	int workPathLen = strlen(fList.workPath);
	LOG_WATCH(srcPath);
	while ((s_dir = readdir(dir)) != NULL)
	{
		if ((strcmp(s_dir->d_name, ".") == 0)
				|| (strcmp(s_dir->d_name, "..") == 0))
		{
			continue;
		}
		if(g_AnalyseLayer->UpdateTask < 3)
		{
			fList.Reset();
			break;
		}
		if ((fList.List.size() >= this->ScanLimitSize) ||
				( fList.FileSize >= 100 * MBYTE ) ) //
		{
			LOG_DEBUG("Count or Size Limit out:ScanFileNum=%d ,ScanFileSize=%d",
					fList.List.size(), fList.FileSize);
			break;
		}
		OS_SNPRINTF(curdir, sizeof(curdir), "%s%s", srcPath, s_dir->d_name);
		stat_flag = stat(curdir, &file_stat);
		if (S_ISDIR(file_stat.st_mode))      //目录名   CD
		{
			ScanPathFileTo(curdir, fList);
			continue;
		}
		//判断文件的修改时间和访问时间同时满足条件后发移动到发送目录中, 由cp复制命令操作的文件修改时间是不会变化的。
		if ((((tNow - file_stat.st_mtime) <= this->stableTime + 2)
				|| ((tNow - file_stat.st_atime) <= this->stableTime))) //最后修改时间和访时间
			continue;

		FILEINFOPACK_HEAD finfo = { 0 };
		memset(&finfo,0,sizeof(finfo));
		if(-1 == stat_flag)         //支持大文件 大于3G的 (在64位系统下没问题，但在32位系统下不行)
		{
			FILE *pfile = fopen64(curdir,"r");
			if(pfile!= NULL)
			{
				fseeko64(pfile, 0, SEEK_END);
				finfo.nFileSize = ftello64(pfile);
				fclose(pfile);
			}
		}
		else
		{
			finfo.nFileSize = file_stat.st_size;
		}  
#endif
	//WIN下目录文件遍历方式
#ifdef OS_WIN
	CFileFind fFind;
	CString szDir = srcPath;
	szDir += "*.*";

	string virType("|");
	virType.append(this->rule.strFileType);
	virType.append("|");
	string suffix;
	bool fix_flag = strlen(this->rule.strFileType) > 0;
	char curdir[PATH_LEN + FILE_NAME_LEN] =
	{ 0 };
	int len = strlen(srcPath);
	 
	time_t tNow = time( NULL);
	int stat_flag = 0;
	int workPathLen = strlen(fList.workPath);
	bool res = fFind.FindFile(szDir); 
	while(res)
	{ 
		res = fFind.FindNextFile();
		if(fFind.IsDirectory() && fFind.IsDots())
			continue;
		if(g_AnalyseLayer->UpdateTask < 3)
		{
			fList.Reset();
			break;
		}
		if ((fList.List.size() >= this->ScanLimitSize) ||
				( fList.FileSize >= 100 * MBYTE ) ) //
		{
			LOG_DEBUG("Count or Size Limit out:ScanFileNum=%d ,ScanFileSize=%d",
					fList.List.size(), fList.FileSize);
			break;
		}
		if(fFind.IsDirectory() && !fFind.IsDots())
		{
			//如果是一个子目录，用递归继续往深一层找
			CString strPath = fFind.GetFilePath();
			char strFilePath[PATH_LEN + FILE_NAME_LEN] =
			{0};
			_snprintf(strFilePath,sizeof(strFilePath), "%s", (LPCTSTR)strPath);
			strFilePath[sizeof(strFilePath)-1] ='\0'; 
			ScanPathFileTo(strFilePath, fList);
			continue;
		} 
		//显示当前访问的文件
		CString _strPath = fFind.GetFilePath(); 
		_snprintf(curdir, sizeof(curdir), "%s", (LPCTSTR)_strPath);
		curdir[sizeof(curdir)-1] = '\0';
		HANDLE hCheckFile = CreateFile (curdir, GENERIC_READ/*|GENERIC_WRITE*/,
				FILE_SHARE_READ|FILE_SHARE_DELETE,NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	    //判断文件是否被占用
		if (hCheckFile == INVALID_HANDLE_VALUE)
		{
			LOG_WARN("errorno:%d",GetLastError());
			LOG_WARN("file is being occupied!!!,hCheckFile == INVALID_HANDLE_VALUE,file:%s",curdir);
			continue;
		} 
		CloseHandle(hCheckFile);
				 
		time_t tNow = time(NULL);
		struct _stat64 file_stat;              //支持大文件 大于3G的
		_stat64(curdir, &file_stat);
		//判断文件的修改时间和访问时间同时满足条件后发移动到发送目录中, 由cp复制命令操作的文件修改时间是不会变化的。
		if ((((tNow - file_stat.st_mtime) <= this->stableTime + 2)
				|| ((tNow - file_stat.st_atime) <= this->stableTime))) //最后修改时间和访时间
			continue;
			
		FILEINFOPACK_HEAD finfo = { 0 };
		memset(&finfo,0,sizeof(finfo));
		finfo.nFileSize = file_stat.st_size;
#endif

		if(fix_flag)
		{
			suffix.append(curdir);
			suffix.erase(0,suffix.find_last_of('.') + 1);
			suffix.insert(0,"|");
			suffix.append("|");

			if(virType.find(suffix) != string::npos)
				continue;
		}

		fList.FileSize += file_stat.st_size;
		finfo.nTaskID = this->nTaskID;
		finfo.mtime = tNow - file_stat.st_mtime;
		finfo.atime = tNow - file_stat.st_atime;
		OS_SNPRINTF(finfo.strFilePath,sizeof(finfo.strFilePath), "%s", fList.workPath); 
		OS_SNPRINTF(finfo.strFileName,sizeof(finfo.strFileName), "%s", curdir+workPathLen);
		fList.List.push_back(finfo);
		SLEEP_US(Config::Sender.nIOSleep); 
	}
#ifdef OS_LINUX
	closedir(dir);
#endif
#ifdef OS_WIN
	fFind.Close(); //关闭
#endif
	return 0;
}
/*******************************************************************************
 * Function     : FileTask::system_move
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int system_move(const char *oldname, const char* newname){
#ifdef OS_WIN
	if ( NULL == oldname || NULL == newname)
	{
		printf("入参错误！\n");
		return -1;
	}
	char cmd[1024 * 2] =
	{ 0 };
	sprintf(cmd, "sudo mv -f '%s' '%s'", oldname, newname);
	int status = system(cmd);
    if (status != -1)// && WIFEXITED(status) && 0==WEXITSTATUS(status))
	{
        return 0;
    }
    LOG_WARN("移动文件失败,%s",oldname);
	return -1;
#endif
#ifdef OS_LINUX 
	ifstream source(oldname, ios::in|ios::binary);
	ofstream target(newname, ios::trunc|ios::out|ios::binary);
	if (source.is_open() && target.is_open()){
		target << source.rdbuf();
	}
	target.close();
	source.close();
	struct stat64 ofile_stat;
	struct stat64 nfile_stat;
	stat64(newname, &nfile_stat);
	stat64(oldname, &ofile_stat);
	if(nfile_stat.st_size != ofile_stat.st_size)
	{
		LOG_WARN("error move file,%s",oldname);
		remove(newname);
		return -1;
	}
	remove(oldname);
#endif
	return 0;
}
/*******************************************************************************
 * Function     : FileTask::Rename
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
bool FileTask::Rename(const char *oldname, const char *newname){

	if(move_fun != rename)
	{
		create_multi_dir(newname);
	}

	if(-1 != move_fun(oldname, newname))
		return true;
	LOG_WARN("[%d]:%s",errno, strerror(errno));

	if (errno == ENOENT)
	{
		if (create_multi_dir(newname) < 0)
		{
			LOG_WARN("failure to create_multi_dir:%s", newname);
			return false;
		}
		LOG_WARN("old:%s,\nnew:%s\n",oldname,newname);
		if(-1 != move_fun(oldname, newname))
			return true;
		LOG_WARN("[%d]:%s",errno, strerror(errno));
		exit(1);
	}
	if (errno == EXDEV)
	{//The two file names newname and oldname are on different file systems.
		//change the function of move file
		LOG_WARN("change the function of move file");
		(move_fun == rename)?(move_fun = system_move):(move_fun = rename);
		if(-1 != move_fun(oldname, newname))
			return true;
		LOG_WARN("[%d]:%s",errno, strerror(errno));
	}
	if(os_access_fok(newname,0) == 0)
		return true;
	return false;
}
/*******************************************************************************
 * Function     : FileTask::MoveFileListToTemp
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::MoveFileToTemp(void){

	string strDstFile;
	string strSrcFile;
	LOG_DEBUG("准备将%d个文件挪动Temp目录下,一共%d byte",this->TaskPath.List.size(),this->TaskPath.FileSize);
	for (auto file : this->TaskPath.List)
	{ 
		strSrcFile = string(file.strFilePath) +  file.strFileName;
		strDstFile = string(this->TempPath.workPath) + file.strFileName;

		if (!Rename(strSrcFile.c_str(), strDstFile.c_str()))
		{
			SLEEP_MS(10);
			LOG_WARN("false to move file :%s to %s", file.strFileName, strDstFile.c_str());
			continue;
		}
		OS_SNPRINTF(file.strFilePath,sizeof(file.strFilePath), "%s", this->TempPath.workPath);

	    std::unique_lock <std::mutex> lck(tsk_Mutex);
		this->TempPath.List.push_back(file);
		this->TempPath.FileSize += file.nFileSize;
	    lck.unlock();
		SLEEP_US(Config::Sender.nIOSleep);
	}
	LOG_DEBUG("挪动完毕,当前Temp目录中有%d个文件,%d byte等待发送",
			this->TempPath.List.size(),this->TempPath.FileSize);
	this->TempPath.Flag = 2;
	this->TaskPath.Reset();
}
/*******************************************************************************
 * Function     : FileTask::ScanAndMoveFile
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::ScanAndMoveFile(void){
	if (SWGAP_SND != this->transMod || this->rule.status == 0)
	{
		return;
	}
	if ((time(NULL) - this->tLastSync) < Config::Sender.nWait)
	{
		return;
	}
	LOG_DEBUG("Begin to Scan file ...");
	this->tLastSync = time(NULL);

	if (0 == this->TaskPath.Flag)
	{
		this->TaskPath.FileSize = 0;
		this->ScanPathFileTo(this->TaskPath.workPath, this->TaskPath);
		if (0 < this->TaskPath.List.size())
		{
			this->TaskPath.Flag = 2;
		}
	}
	if (2 == this->TaskPath.Flag
			&& 0 == this->TempPath.Flag)
	{
		this->MoveFileToTemp();
	}
}

/*******************************************************************************
 * Function     : FileTask::Send1TaskFile
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::SendFile(void){ 
    if (this->rule.status == 0)
    {
        LOG_TRACE("Task:%d status == 0", this->rule.nTaskID);
		return;
    }
    if (SWGAP_RCV == this->transMod)
    {
        LOG_TRACE("Task:%d,SWGAP_SND != GetTransDirect", this->rule.nTaskID);
		return;
    }
    this->SendTaskFile();
}
/*******************************************************************************
 * Function     : FileTask::SendTaskFile
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::SendTaskFile(void)
{
	int res_send = 0;
	if (2 != this->TempPath.Flag && 2 != this->SendBuf.Flag)
	{
		return;
	}
	if (0 == this->SendBuf.Flag)
	{
		this->SendBuf.Flag = 2;
	    std::unique_lock <std::mutex> lck(tsk_Mutex);
		auto lbegin = this->TempPath.List.begin();
		auto lend = this->TempPath.List.end();
		if(this->TempPath.List.size() > this->rule.nLimitSpeed)
		{
			lend = lbegin;
			std::advance(lend, this->rule.nLimitSpeed);
		}
		std::copy(lbegin, lend, std::back_inserter(this->SendBuf.List));
		this->TempPath.List.erase(lbegin, lend);
		this->SendBuf.FileSize = 0;
		for(auto itfile: this->SendBuf.List)
		{
			this->SendBuf.FileSize += itfile.nFileSize;
			this->TempPath.FileSize -= itfile.nFileSize;
		}
		this->TempPath.Flag = 0; 
	}
	if (2 != this->SendBuf.Flag)
	{
		if (0 == this->SendBuf.List.size())
		{
			LOG_WATCH(this->SendBuf.List.size());
			return;
		}
	}
	auto totalSize = BYTE2KB(this->SendBuf.FileSize);
	this->TaskRespState = 0;
    this->CurSend.reset();
	LOG_INFO("Request a task,num=%d,size=%dKB"
			,this->SendBuf.List.size(),totalSize);
	res_send = this->SendFileTaskReq(1, totalSize);
	if (0 > res_send || 0 > this->GetSendFileTaskRsp(true))
	{
        LOG_WARN("Failed to req a task to send file,num=%d,size=%dKB"
        		,this->SendBuf.List.size(),totalSize);
        this->SendBuf.Flag = 2;
		return;//发送不成功，跳出，等待下一
	}
	if( this->SendBuf.List.empty())
	{
		this->SendBuf.Reset();
		return;
	}
	time_t t1 = time(NULL);
	this->TaskRespState = 0;
	res_send = this->SendTempFileFromBuf();
	if (0 > res_send)
	{
		LOG_WARN("发送失败等待下次发送");
		this->SendBuf.Flag = 2;
		return;//发送不成功，跳出，等待下一
	}
	else if (0 == res_send && 0 > this->WaitResendFailFile(true))
	{
		LOG_WARN("未返回等待下次继续发送");
		this->SendBuf.Flag = 2;
		return;	//发送不成功，跳出，等待下一
	}

    if (this->CurSend.TotalSize >= MBYTE)
        LOG_INFO("Task send finished! SendFiles=%d time=%ds TotalSize=%dMB\n"
        		, this->CurSend.Files, (int) (time(NULL) - t1), BYTE2MB(this->CurSend.TotalSize));
    else  if (this->CurSend.TotalSize >= KBYTE)
            LOG_INFO("Task send finished! SendFiles=%d time=%ds TotalSize=%dKB\n"
            		, this->CurSend.Files, (int) (time(NULL) - t1), BYTE2KB(this->CurSend.TotalSize));
    else
        LOG_INFO("Task send finished! SendFiles=%d time=%ds TotalSize=%dByte\n"
        		, this->CurSend.Files, (int) (time(NULL) - t1), this->CurSend.TotalSize);
	if (this->SendBuf.List.empty())
	{
		this->SendBuf.Reset();
	}
	else
	{
		this->SendBuf.Flag = 2;
	}
}

/*******************************************************************************
 * Function     : FileTask::GetSendFileTaskRsp
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::GetSendFileTaskRsp(bool bwait){

	if (1 > this->TaskRespState)
	{
		if (bwait)
		{
			std::unique_lock <std::mutex> lck(tsk_Mutex);
			tsk_Cond.wait_for(lck, std::chrono::seconds(20));
			LOG_DEBUG("唤醒后:this->m_TaskRespState = %d\n",
					this->TaskRespState);
		}
		else
		{
			LOG_WARN("Req task timeout!");
			return -1;
		}
	}else{
		LOG_WATCH(this->TaskRespState);
		this->TaskRespState = 0;
		return 0;
	}
	return GetSendFileTaskRsp(false);
}
/*******************************************************************************
 * Function     : FileTask::SendFileData
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
const int FILE_BUF_MAXSIZE = RW_SOCKT_BUF_MINLEN_;//这里不能超过4M，网闸中间代理GapServer有限制
char *dataBuf = new char[FILE_BUF_MAXSIZE];
int FileTask::SendFileData(FILEINFOPACK_HEAD &finfo, int sid){
	finfo.nSessionId = sid;
	int nRev = 0;
	string fdir = string(finfo.strFilePath) + finfo.strFileName;
#ifdef OS_LINUX
	FILE *file = fopen64(fdir.c_str(), "r");
#else
	FILE *file = fopen(fdir.c_str(),"rb");
#endif

	if ( NULL == file)   //打开失败，不同步，要将该信息上报，并写日志库
	{
		LOG_WARN("Open File: %s  errno:%d  error:%s ",
				fdir.c_str(), errno, strerror(errno));
		if (2 == errno)
			return -1;
		else
			return -2;
	}
	
	FILEINFOPACK_HEAD FileInfo =
	{ 0 };
	
#ifdef OS_LINUX 
	struct stat file_stat;
	int res_stat =
		stat(fdir.c_str(), &file_stat);
	if(-1 == res_stat)         //支持大文件 大于3G的 (在64位系统下没问题，但在32位系统下不行)
	{
		fseeko64(file, 0, SEEK_END);
		FileInfo.nFileSize = ftello64(file);
	}
	else
	{
		FileInfo.nFileSize =  file_stat.st_size;
	}
#endif
	
#ifdef OS_WIN 
		time_t tNow = time(NULL);
		struct _stat64 file_stat;              //支持大文件 大于3G的
		_stat64(fdir.c_str(), &file_stat);
		FileInfo.nFileSize =  file_stat.st_size;
#ifdef FILE_MARK
	//最终的长度要减去标记头长度的
	FileInfo.nFileSize = FileInfo.nFileSize-HEAD_SIZE;
#endif
#endif 
	memcpy(&FileInfo,&finfo,sizeof(FileInfo));
	LOG_INFO("[%d,size=%ld]:%s",FileInfo.nSessionId,FileInfo.nFileSize,FileInfo.strFileName);//zzdd
	if (this->SendFileInfo(FileInfo) < 0)
	{
		LOG_WARN("网络发送失败，可确认网络连接Socket已断开");
		fclose(file);
	}
	this->SendFileSyncLog((char*) "", FileInfo, 1, (char*) "文件请求成功");

#ifdef OS_LINUX
	fseek(file, 0, SEEK_SET);
#else
#ifdef FILE_MARK
	fseek(file, HEAD_SIZE, SEEK_SET);
#else
	fseek(file, 0, SEEK_SET);
#endif
#endif
	int nSend = 0;
	int ntfsend = 0;  
	while ((nRev = fread(dataBuf, 1, FILE_BUF_MAXSIZE, file)) > 0) //读磁盘文件
	{
		if ((nSend = this->SendFileData(sid, dataBuf, nRev)) < 0)
		{
			fclose(file);
			return -1;
		}
		memset(dataBuf,0,nRev);
		ntfsend += nRev;
		if(this->rule.nLinkFileCheckId
				&& this->rule.nLinkVirusRuleId)
		{
			LOG_WATCH(nRev);//60*1000
			LOG_INFO("[%d/%lld]wait %d ms virus check...",ntfsend,FileInfo.nFileSize,(int)(nRev/1.5));
			SLEEP_MS((nRev/1.5/2));
			g_AnalyseLayer->bCheckState = false;
	    	SLEEP_MS((nRev/1.5/2));
			g_AnalyseLayer->bCheckState = true;
		}
			SLEEP_US(Config::Sender.nIOSleep);
	}
	if (sid == 0)
	{
		this->CurSend.TotalSize += finfo.nFileSize;
	}
	if (this->SendFileFinished(FileInfo.nSessionId, 0) < 0)
	{
		LOG_WARN("SendFileFinished( FileInfo.nSessionId , 0 ) failed\n");
		fclose(file);
		return -2;
	}
	this->CurSend.Files++;
	fclose(file);
	this->SendFileSyncLog((char*) "", FileInfo, 1, (char*) "文件发送结束");
	return ntfsend;
}
/*******************************************************************************
 * Function     : FileTask::WaitResendFailFile
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::WaitResendFailFile(bool bwait){

	LOG_WATCH(this->TaskRespState);
	if (0 == this->TaskRespState)
	{
		if (bwait)
		{
			if (0 > this->SendTaskFinished(0))
			{
				LOG_WARN("send task finish failed!");
				return -1;    //等待重发
			}
			std::unique_lock <std::mutex> lck(tsk_Mutex);
			tsk_Cond.wait_for(lck, std::chrono::seconds(60));
			LOG_DEBUG("唤醒后:this->m_TaskRespState = %d\n",
					this->TaskRespState);
		}
		else
		{
			LOG_WARN("服务端没有返回,继续等待服务端返回,不发送TaskFinished 2!");
			return -1;
		}
	}else if(this->TaskRespState == 2){
		this->TaskRespState = 0;
		return 0;
	}
	else if(this->TaskRespState == 1)
	{
		this->SendBuf.FileSize = 0;
		auto &reSendFile = this->CurRecv.ResendList;
		auto &reFuseFile = this->CurRecv.RefuseList;
		auto itRSFile = reFuseFile.begin();
		char loginfo[500] = {0};

		auto iter = this->SendBuf.List.begin();
		LOG_DEBUG("正在删除文件");
		for (; iter != this->SendBuf.List.end();)
		{
			itRSFile = std::find_if(reFuseFile.begin(),reFuseFile.end(),[&](FILE_SESSIONID id){
				return id == iter->nSessionId;
			});
			string fname(iter->strFilePath); 
			fname += iter->strFileName ;
			if(itRSFile != reFuseFile.end())
			{
				LOG_WARN("Virus File:%s",fname.c_str());
				memset(loginfo, 0, sizeof(loginfo));
				sprintf(loginfo,"文件核查不通过,检测到敏感关键字或病毒:%s",iter->strFileName);
				this->SendFileAlarmcLog(*iter,2,0,loginfo);
				string strDstFile = string(Config::Sender.strVirusPath) + iter->strFileName;
				Rename(fname.c_str(), strDstFile.c_str());
				reFuseFile.erase(itRSFile);
				this->SendBuf.List.erase(iter++);
				continue;
			}
			itRSFile = find(reSendFile.begin(),reSendFile.end(),iter->nSessionId);
			if(itRSFile != reSendFile.end())
			{
				reSendFile.erase(itRSFile);
				this->SendBuf.FileSize += iter->nFileSize;
				iter++;
				continue;
			}
			else if (Config::Sender.Backup)
			{
				char dstfile[1024] = {0};
				sprintf(dstfile, "%s/%d/%s",Config::Sender.strBackup,nTaskID,
					iter->strFileName + strlen(iter->strFilePath));
				Rename(fname.c_str(), dstfile);
			}
			else if (0 != remove(fname.c_str()))//删除文件失败
			{
//				LOG_ERROR("del errorno:%d,%s,:%s",
//						errno, strerror(errno), fname.c_str());
			}
			this->SendBuf.List.erase(iter++);
			SLEEP_US(Config::Sender.nIOSleep);
		}
		reSendFile.clear();
		reFuseFile.clear();
		this->TaskRespState = 0;
		return 0;
	}
	return WaitResendFailFile( false);
}

/*******************************************************************************
 * Function     : FileTask::SendFileTaskReq
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::SendFileTaskReq(int nLevel, int nNeedSpace)
{
    GAPPACK_HEADER head;
    char SendBuf[sizeof(GAPPACK_HEADER) + 20];
    int offset = 0;
    this->GetHeadPackValue(CMD_TYPE_FILE, CMD_FILE_SEND_TASK_REQ, sizeof(int) * 4, head);
    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);

    memcpy(SendBuf + offset, &this->nTaskID, sizeof(this->nTaskID));
    offset += sizeof(this->nTaskID);
    memcpy(SendBuf + offset, &nLevel, sizeof(nLevel));
    offset += sizeof(nLevel);
    memcpy(SendBuf + offset, &nNeedSpace, sizeof(nNeedSpace));
    offset += sizeof(nNeedSpace);
    int size = 0;
    memcpy(SendBuf + offset, &size, sizeof(size));
    offset += sizeof(size);
    this->Buffer.reset();
    return this->Send(SendBuf, head.TotalLength,false);
}
/*******************************************************************************
 * Function     : FileTask::SendTaskFinished
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::SendTaskFinished(int nResult)
{
    GAPPACK_HEADER head;
    char SendBuf[sizeof(GAPPACK_HEADER) + 20] = { 0 };
    int offset = 0;
    this->GetHeadPackValue(CMD_TYPE_FILE, CMD_FILE_FINISHED_TASK_REQ, sizeof(int) * 2, head);
    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);
    memcpy(SendBuf + offset, &this->nTaskID, sizeof(this->nTaskID));
    offset += sizeof(this->nTaskID);
    memcpy(SendBuf + offset, &nResult, sizeof(nResult));
    offset += sizeof(nResult);
    return this->Send( SendBuf, head.TotalLength,false);
}

/*******************************************************************************
 * Function     : FileTask::SendFileInfo
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::SendFileInfo(FILEINFOPACK_HEAD fInfo)
{
    GAPPACK_HEADER head;
    char SendBuf[sizeof(GAPPACK_HEADER) + sizeof(FILEINFOPACK_HEAD) + 20] = { 0 };
    int offset = 0;
    this->GetHeadPackValue(CMD_TYPE_FILE, CMD_FILE_SEND_FILE_INFO_REQ,
    		sizeof(FILEINFOPACK_HEAD) + sizeof(this->nTaskID), head);
	memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
	offset += sizeof(GAPPACK_HEADER);
	memcpy(SendBuf + offset, &this->nTaskID, sizeof(this->nTaskID));
	offset += sizeof(this->nTaskID);
    memcpy(SendBuf + offset, &fInfo, sizeof(FILEINFOPACK_HEAD));
    offset += sizeof(FILEINFOPACK_HEAD);
    return this->Send(SendBuf, head.TotalLength,true);
}
/*******************************************************************************
 * Function     : FileTask::SendFileData
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::SendFileData(int nSessionID, char *buf, int nDataLen)
{
    if (NULL == buf)
        return -1;

    GAPPACK_HEADER head;
    int buflen = sizeof(GAPPACK_HEADER) + nDataLen + 20;
    char *SendBuf = (char*) malloc(buflen);
    int offset = 0;

    memset(SendBuf, 0, buflen);
    this->GetHeadPackValue(CMD_TYPE_FILE, CMD_FILE_SEND_FILE_DATA,
    		nDataLen + sizeof(this->nTaskID) + sizeof(nSessionID), head);
    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);
    memcpy(SendBuf + offset, &this->nTaskID, sizeof(this->nTaskID));
    offset += sizeof(this->nTaskID);
    memcpy(SendBuf + offset, &nSessionID, sizeof(nSessionID));
    offset += sizeof(nSessionID);
    memcpy(SendBuf + offset, buf, nDataLen);
    offset += nDataLen;
    int len = this->Send( SendBuf, head.TotalLength,true);
	free(SendBuf);
	SendBuf = NULL;
	return len;
}
/*******************************************************************************
 * Function     : FileTask::SendFileFinished
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::SendFileFinished(int nSessionID, int nFlag)
{
    GAPPACK_HEADER head;
    char SendBuf[sizeof(GAPPACK_HEADER) + 20];
    int offset = 0;
    this->GetHeadPackValue(CMD_TYPE_FILE, CMD_FILE_SEND_FILE_FINISHED_REQ,
    		sizeof(int) * 3, head);
    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);
    memcpy(SendBuf + offset, &this->nTaskID, sizeof(this->nTaskID));
    offset += sizeof(this->nTaskID);
    memcpy(SendBuf + offset, &nSessionID, sizeof(int));
    offset += sizeof(int);
    memcpy(SendBuf + offset, &nFlag, sizeof(int));
    offset += sizeof(int);

    return this->Send(SendBuf, head.TotalLength,true);
}
/*******************************************************************************
 * Function     : FileTask::OnRevFileSendTaskReq
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::OnRevFileSendTaskReq(char *buf, int nlen)
{
    if (NULL == buf)
        return;
    int offset = 0;
    int DiskIsOK = -1;           //初始化为不允许传输
    int taskID = 0;
    int nLevel = 0;
    int size = 0;
    int rcvfsize = 0;

    memcpy(&taskID, buf + offset, sizeof(taskID));
    offset += sizeof(taskID);
    memcpy(&nLevel, buf + offset, sizeof(nLevel));
    offset += sizeof(nLevel);
    memcpy(&rcvfsize, buf + offset, sizeof(rcvfsize));
    offset += sizeof(rcvfsize);
    memcpy(&size, buf + offset, sizeof(size));
    offset += sizeof(size);

	CurRecv.reset();
	if (rcvfsize >= 1024)
	{
		LOG_INFO("RevDownload Req TaskID =%d  level=%d   nTotalFileSize=%dMB",  taskID , nLevel , rcvfsize / (1024));
	}
	else
	{
		LOG_INFO("RevDownload Req TaskID =%d  level=%d   nTotalFileSize=%dKB",  taskID , nLevel , rcvfsize);
	}

    //在这里要检查磁盘空间状况
	unsigned long int nFreeSize = GetDiskFreeSpace(this->rule.strSrcPath);
	if ((nFreeSize * 1024 - rcvfsize) > 50 * 1024)      //以K为单位,保存该目录后，还应有大于50M的磁盘空间
	{
		DiskIsOK = 1;
	}
	else
	{
		DiskIsOK = -1;
		LOG_WARN("disk full!!!!!!!!");
		SLEEP_S(10);           //无可用的磁盘空间，等待10秒，,避免过频同步
	}

    GAPPACK_HEADER head;
    const int datasize = sizeof(taskID) + sizeof(DiskIsOK);
    char SendBuf[sizeof(GAPPACK_HEADER) + datasize];
    offset = 0;
    GetHeadPackValue(CMD_TYPE_FILE, CMD_FILE_SEND_TASK_RSP, datasize, head);
    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);
    memcpy(SendBuf + offset, &taskID, sizeof(taskID));
    offset += sizeof(taskID);
    memcpy(SendBuf + offset, &DiskIsOK, sizeof(DiskIsOK));
    offset += sizeof(DiskIsOK);

    Send( SendBuf, head.TotalLength);
    CurRecv.ResendList.clear();
    CurRecv.RefuseList.clear();
}
/*******************************************************************************
 * Function     : OnRevFileSendTaskRsp
 * Input        :
 * Return       :
 * Description  :
 * Created on   : 2018年2月5日
 * Author       : luzxiang
 * Notes        : --
 *******************************************************************************/
void FileTask::OnRevFileSendTaskRsp(char *buf, int nlen)
{
    int taskID = -1;
    int nDiskFlag = -1;
    int size = 0;
    int offset = 0;
    memcpy(&taskID, buf, sizeof(taskID));
    offset += sizeof(taskID);
    memcpy(&nDiskFlag, buf + offset, sizeof(nDiskFlag));
    offset += sizeof(nDiskFlag);
    memcpy(&size, buf + offset, sizeof(size));
    offset += sizeof(size);
	LOG_WATCH(size);
    this->CurRecv.RefuseList.clear();
    TaskRespState = nDiskFlag;
	this->tsk_Cond.notify_all(); // 唤醒所有线程.
}
/*******************************************************************************
 * Function     : FileTask::OnRevFileSendFileInfoReq
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::OnRevFileSendFileInfoReq(char *buf, int nlen)
{
    if (NULL == buf)
        return;
    FILEINFOPACK_HEAD fileInfo = { 0 };
    int taskid = 0;
    int offset = 0;
    memcpy(&taskid, buf, sizeof(taskid));
    offset += sizeof(taskid);
    memcpy(&fileInfo, buf+offset, sizeof(fileInfo));
    offset += sizeof(fileInfo);
    memcpy(&CurFile.Info, &fileInfo, sizeof(CurFile.Info));
    offset += sizeof(CurFile.Info);
	os_format2path(fileInfo.strFileName,sizeof(fileInfo.strFileName));

    if (this->CurFile.pHandle)
    {
        fclose(this->CurFile.pHandle);
        this->CurFile.pHandle = NULL; 
    } 
    if (CurFile.Info.nTaskID != this->nTaskID)
    {
    	LOG_WATCH(fileInfo.strFileName);
        LOG_WARN("m_CurFileInfo.nTaskID:%d != nTaskID:%d", CurFile.Info.nTaskID, nTaskID);
        return;
    }
	this->CurFile.RecvSize = 0; 
    char strFileName[PATH_LEN + FILE_NAME_LEN] = { 0 };
	OS_SNPRINTF(strFileName,sizeof(strFileName),"%s%s",
		this->rule.strSrcPath,fileInfo.strFileName);
	 
#ifdef OS_LINUX
    char strOutTemp[PATH_LEN + FILE_NAME_LEN] = { 0 };
    char strfile[PATH_LEN + FILE_NAME_LEN] = { 0 };
    OS_SNPRINTF(strfile, sizeof(strfile), "%s", strFileName);
    //在接收端转换编码
    int nRet = GbkToUtf8ForLinux(strfile, sizeof(strfile), strOutTemp, sizeof(strOutTemp));
    if (nRet == 0)
    {
        OS_SNPRINTF(strFileName, sizeof(strFileName), "%s", strOutTemp);
        LOG_WATCH(strFileName);
    }
    LOG_WATCH(strFileName);
#endif

#ifdef OS_WIN
    if(IsTextUTF8(strFileName, strlen(strFileName)))
    {
        //在接收端转换编码
        char *pstrFile = UTF8ToGBK(strFileName);
        if(pstrFile)
        {
            strncpy_s(strFileName, pstrFile, sizeof(strFileName));
            strFileName[sizeof(strFileName)] = '\0';
            delete pstrFile;
        }
    }
#endif

    if(os_access_fok(strFileName, 0) >= 0)
    {
        os_remove(strFileName); //如果文件存在则删除该文件
    }
    create_multi_dir(strFileName);
    sprintf(CurFile.Info.strFileName, "%s", strFileName);
#ifdef OS_LINUX
    this->CurFile.pHandle = fopen64(CurFile.Info.strFileName, "w"); //);//|O_TRUNC); // 如果已存在则作覆盖并清零处理(清零处理耗资源)
#endif
#ifdef OS_WIN
    this->CurFile.pHandle = fopen(CurFile.Info.strFileName, "wb");        //);//|O_TRUNC); // 如果已存在则作覆盖并清零处理(清零处理耗资源)
#endif

}
//
/*******************************************************************************
 * Function     : FileTask::OnRevFileSendFileInfoRsp
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::OnRevFileSendFileInfoRsp(char *buf, int nlen)
{
    if (NULL == buf)
        return;
    int taskID = 0;
    int TaskFlag = 0;
    memcpy(&taskID, buf, sizeof(taskID));
    memcpy(&TaskFlag, buf + sizeof(taskID), sizeof(taskID));
    FileRespState = TaskFlag;
}

/********************************************************************************
 Function:       OnRevFileData
 Description:    接收文件数据（内端）
 Calls:
 Table Accessed:
 Table Updated:
 Input:          buf   收到的数据缓冲区
 nlen  收到的数据长度
 Output:
 Return:
 Others:
 *********************************************************************************/
void FileTask::OnRevFileData(char *buf, int nlen)
{
    if (NULL == buf)
        return;

    unsigned int nSessionID = 0;
    int taskID = 0;
    int offset = 0;
    memcpy(&taskID , buf+offset, sizeof(taskID));
    offset += sizeof(taskID);
    memcpy(&nSessionID, buf+offset, sizeof(nSessionID));
    offset += sizeof(nSessionID);
    char loginfo[100] = {0};
    if(CurFile.Info.nSessionId == 0)
    	return;
    if (nSessionID != CurFile.Info.nSessionId)
    {
        UpdateResendFileList(nSessionID);
        if (this->CurFile.pHandle != NULL)
            fclose(this->CurFile.pHandle);
        this->CurFile.pHandle = NULL;
        sprintf(loginfo,
        		"Failed to receive,SessionID is not same,CurId[%d]!=reqID[%d]",
        		CurFile.Info.nSessionId, nSessionID);

        this->SendFileSyncLog((char*) "", CurFile.Info, 0, loginfo);
        LOG_WARN("m_CurFileInfo.nSessionId:%d != nSessionID:%d."
        		"current file:%s",
                CurFile.Info.nSessionId, nSessionID,
				CurFile.Info.strFileName);
        CurFile.RecvSize = 0;
        CurFile.Info.nSessionId = 0;
        return;
    }
    if (this->CurFile.pHandle == NULL)        //
    {
        sprintf(loginfo,
        		"Failed to receive,do not receive file info but data");
        UpdateResendFileList(CurFile.Info.nSessionId);
        this->SendFileSyncLog((char*) "", CurFile.Info, 0, loginfo);
        LOG_WARN("req to resend file, sessionId:%d", CurFile.Info.nSessionId);
        LOG_WARN("%s",loginfo);
        CurFile.RecvSize = 0;
        CurFile.Info.nSessionId = 0;
        return;
    }
    fwrite(buf + offset, nlen - offset, 1, this->CurFile.pHandle);
    CurFile.RecvSize += nlen - sizeof(nSessionID);
	SLEEP_US(10);
}
//
/*******************************************************************************
 * Function     : FileTask::OnRevSendFileFinishedReq
 * Description  : 接收到文件发送完成包（请求文件验证）
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::OnRevSendFileFinishedReq(char *buf, int nlen)
{
    if (NULL == buf)
        return -1;

    unsigned int nSessionID = 0, nResultCode = 0;
    int taskID = 0;
    int offset = 0;
    memcpy(&taskID , buf + offset, sizeof(taskID));
    offset += sizeof(taskID); 
    memcpy(&nSessionID, buf + offset, sizeof(nSessionID));
    offset += sizeof(nSessionID);
    memcpy(&nResultCode, buf + offset, sizeof(nResultCode));

	if (this->CurFile.pHandle)
	{
		fclose(this->CurFile.pHandle);
		this->CurFile.pHandle = NULL;
	}
	if(nResultCode == 1)//refuse
	{
        UpdateRefuseFileList(nSessionID);
        LOG_WATCH(CurFile.Info.strFileName);
        string fdir(CurFile.Info.strFilePath);
        fdir.append("/");
        fdir.append(CurFile.Info.strFileName);
        remove(fdir.c_str());
        return 0;
	}
    if(CurFile.RecvSize < CurFile.Info.nFileSize)
    {
    	LOG_WARN("CurFileRecvTotalSize=%d < m_CurFileInfo.nFileSize=%d",
    			CurFile.RecvSize , CurFile.Info.nFileSize);
        UpdateResendFileList(nSessionID);
        return 0;
    }
	CurRecv.Files++;
	this->SendFileSyncLog((char*) "", CurFile.Info, 1, (char*) "文件接收完毕");
    return 0;

} 
/*******************************************************************************
 * Function     : FileTask::OnRevTaskFinishedReq
 * Description  : 接收任务完成消息包(内端)
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::OnRevTaskFinishedReq(char *buf, int nlen)
{
    if (NULL == buf)
        return;
    int taskID = 0;
    int nResultCode = 0;
    memcpy(&taskID, buf, sizeof(taskID));
    memcpy(&nResultCode, buf + sizeof(taskID), sizeof(nResultCode));
//    if (2 == nResultCode)        //不要再发重发请求
//        return;
    if (nResultCode == 0)
    {
		//发送文件重发请求包
		GAPPACK_HEADER head;
		int offset = 0;
		int taskid = taskID;
		auto &reSendFile = CurRecv.ResendList;
		int resendnum = reSendFile.size();
		auto &refuseFile = CurRecv.RefuseList;
		int refuseNum = refuseFile.size();

		const int datalen = sizeof(head) + sizeof(taskid) + sizeof(CurRecv.Files) +
				sizeof(resendnum) + resendnum * sizeof(FILE_SESSIONID)+
				sizeof(refuseNum) + refuseNum * sizeof(FILE_SESSIONID);
		char *SendBuf= new char[datalen];

		GetHeadPackValue(CMD_TYPE_FILE, CMD_FILE_FINISHED_TASK_RSP, datalen, head);

		memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
		offset += sizeof(GAPPACK_HEADER);

		memcpy(SendBuf + offset, &taskid, sizeof(taskid));
		offset += sizeof(taskid);
		memcpy(SendBuf + offset, &CurRecv.Files, sizeof(CurRecv.Files));
		offset += sizeof(CurRecv.Files);

		memcpy(SendBuf + offset, &resendnum, sizeof(resendnum));
		offset += sizeof(resendnum);
		for_each(reSendFile.begin(), reSendFile.end(),[&](FILE_SESSIONID id){
			memcpy(SendBuf + offset, &(id), sizeof(id));
			offset += sizeof(id);
			LOG_DEBUG("%d is resend",id);
		});
		reSendFile.clear();
		memcpy(SendBuf + offset, &refuseNum, sizeof(refuseNum));
		offset += sizeof(refuseNum);
		for_each(refuseFile.begin(), refuseFile.end(),[&](FILE_SESSIONID id){
			memcpy(SendBuf + offset, &(id), sizeof(id));
			offset += sizeof(id);
			LOG_DEBUG("%d is refuse",id);
		});
		refuseFile.clear();
		Send(SendBuf, head.TotalLength);
		delete[] SendBuf;
	}
    if (nResultCode != 0)        //任务不成功则返回( 但须写入日志)//
    {
        LOG_INFO("Task failure to recv nTaskID=%d----", taskID);
    }
    else if (CurFile.RecvSize >= 1*MBYTE)
	{
		LOG_INFO("OnRevTaskFinished nTaskID=%d RevFiles=%d totalSize=%dMB\n",
				taskID , CurRecv.Files , KBYTE2MB(CurFile.RecvSize));
	}
    else if (CurFile.RecvSize >= 1*KBYTE)
	{
		LOG_INFO("OnRevTaskFinished nTaskID=%d RevFiles=%d totalSize=%dKB\n",
				taskID , CurRecv.Files , BYTE2KB(CurFile.RecvSize));
	}
	else
	{
		LOG_INFO("OnRevTaskFinished nTaskID=%d RevFiles=%d totalSize=%dByte\n",
				taskID , CurRecv.Files , CurFile.RecvSize);
	}
}

/*******************************************************************************
 * Function     : SendFileAlarmcLog
 * Description  : TODO
 * Input        : level 告警级别(1:一般告警、2:严重告警、3:紧急告警)
 * 				  type  1:病毒|2:关键字
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::SendFileAlarmcLog(FILEINFOPACK_HEAD &synFile,int level, int type, char*logInfo)
{
    FILESYNC_LOGINFO FileSyncLog = {0};
    time_t tN = time(NULL); struct tm* tm1 = localtime(&tN);

    char logdate[] ="20nndd";
    char* alarmlevel = (level == 1)?((char*)"一般告警"):((level == 2)?((char*)"严重告警"):((char*)"紧急告警"));
    char* alarmtype = (type == 1)?((char*)"病毒"):((level == 2)?((char*)"关键字"):((char*)"病毒|关键字"));
    OS_SNPRINTF(logdate,sizeof(logdate),"%4d%02d",tm1->tm_year + 1900, tm1->tm_mon + 1);

	FileSyncLog.nDirection = Config::SvrInfo.nWorkMode;
    FileSyncLog.nFilesize = synFile.nFileSize;
    FileSyncLog.nTaskID = synFile.nTaskID;
    OS_SNPRINTF(FileSyncLog.strFileName, sizeof(FileSyncLog.strFileName), "%s", synFile.strFileName);
    OS_SNPRINTF(FileSyncLog.strLoginfo, sizeof(FileSyncLog.strLoginfo), "%s", logInfo);
	sprintf(FileSyncLog.strLogtime, "%04d-%02d-%02d %02d:%02d:%02d", tm1->tm_year + 1900, tm1->tm_mon + 1, tm1->tm_mday, tm1->tm_hour,
			tm1->tm_min, tm1->tm_sec);
    OS_SNPRINTF(FileSyncLog.sql,sizeof(FileSyncLog.sql),
            "INSERT INTO "
            "sw_alarm_log_%s"
            "(direction,taskid,module,alarmlevel,alarmtype,logtime,alarmloginfo)"
            "VALUES"
            "('%d','%d','%s','%s','%s','%s','%s');"
            ,logdate, FileSyncLog.nDirection,FileSyncLog.nTaskID,
			"Client文件交换", alarmlevel
            ,alarmtype, FileSyncLog.strLogtime, FileSyncLog.strLoginfo
            );

    GAPPACK_HEADER head;
    char SendBuf[sizeof(GAPPACK_HEADER) + sizeof(FILESYNC_LOGINFO) + 10] = { 0 };
    int offset = 0;
    GetHeadPackValue(CMD_TYPE_LOG, CMD_LOG_FILESYNC, sizeof(FILESYNC_LOGINFO), head);

    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);
    memcpy(SendBuf + offset, &FileSyncLog, sizeof(FILESYNC_LOGINFO));
    offset += sizeof(FILESYNC_LOGINFO);
    Send( SendBuf, head.TotalLength);
}
/*******************************************************************************
 * Function     : CSslFileTask::SendFileSyncLogToManger
 * Input        : status:1-TaskReq 2-TaskRsp 3-FileInfoReq
 * 						 4-FileInfoRsp 5-FileData 6-FileFinishedReq 7-TaskFinished
 * Return       :
 * Description  : 向管理机发送日志
 * Created on   : 2018年1月30日
 * Author       : luzxiang
 * Notes        : --
 *******************************************************************************/
void FileTask::SendFileSyncLog(char*dir, FILEINFOPACK_HEAD &synFile, int status, char*logInfo)
{
    FILESYNC_LOGINFO FileSyncLog = { 0 };
    time_t tN = time(NULL);
	struct tm* tm1 = localtime(&tN);
//	FileSyncLog.nDirection = GapServerInfo.nWorkMode;
	sprintf(FileSyncLog.strLogtime, "%04d-%02d-%02d %02d:%02d:%02d", tm1->tm_year + 1900, tm1->tm_mon + 1, tm1->tm_mday, tm1->tm_hour,
			tm1->tm_min, tm1->tm_sec);

    OS_SNPRINTF(FileSyncLog.strPath, sizeof(FileSyncLog.strPath), "%s", dir);
	FileSyncLog.nDirection = Config::SvrInfo.nWorkMode;
    FileSyncLog.nStatus = status;
    FileSyncLog.nFilesize = synFile.nFileSize;
    FileSyncLog.nTaskID = synFile.nTaskID;
    OS_SNPRINTF(FileSyncLog.strFileName, sizeof(FileSyncLog.strFileName), "%s", synFile.strFileName);
    OS_SNPRINTF(FileSyncLog.strLoginfo, sizeof(FileSyncLog.strLoginfo), "%s", logInfo);

    char strdate[] ="20nndd";
    OS_SNPRINTF(strdate,sizeof(strdate),"%4d%02d",tm1->tm_year + 1900, tm1->tm_mon + 1);

    OS_SNPRINTF(FileSyncLog.sql,sizeof(FileSyncLog.sql),
            "INSERT INTO "
            "sw_filechange_log_%s"
            "(direction,taskid,path,filename,filesize,logintime,loginfo,status)"
            "VALUES"
            "(%d,%d,'%s','%s',%d,'%s','%s',%d);"
            ,strdate
            ,FileSyncLog.nDirection,FileSyncLog.nTaskID,FileSyncLog.strPath,FileSyncLog.strFileName
            ,FileSyncLog.nFilesize,FileSyncLog.strLogtime,FileSyncLog.strLoginfo,FileSyncLog.nStatus
            );
    char strInTemp[PATH_LEN+FILE_NAME_LEN];
    memset(strInTemp, 0, sizeof(strInTemp)); 
    strcpy(strInTemp, FileSyncLog.strFileName);
	if(!IsTextUTF8(strInTemp,sizeof(strInTemp)))
    {
		char *strOutTemp = GBKToUTF8(strInTemp);
        OS_SNPRINTF(FileSyncLog.strFileName, sizeof(FileSyncLog.strFileName), "%s", strOutTemp);
		if(strOutTemp)
			delete strOutTemp;
        LOG_WATCH(FileSyncLog.strFileName);
	} 
#ifdef OS_WIN
    memset(strInTemp, 0, sizeof(strInTemp)); 
    strcpy(strInTemp, FileSyncLog.sql);
	if(!IsTextUTF8(strInTemp,sizeof(strInTemp)))
    {
		char *strOutTemp = GBKToUTF8(strInTemp);
        OS_SNPRINTF(FileSyncLog.sql, sizeof(FileSyncLog.sql), "%s", strOutTemp);
		if(strOutTemp)
			delete strOutTemp;
        LOG_WATCH(FileSyncLog.sql);
	} 
#endif
    GAPPACK_HEADER head;
    char SendBuf[sizeof(GAPPACK_HEADER) + sizeof(FILESYNC_LOGINFO) + 10] = { 0 };
    int offset = 0;
    GetHeadPackValue(CMD_TYPE_LOG, CMD_LOG_FILESYNC, sizeof(FILESYNC_LOGINFO), head);

    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);
    memcpy(SendBuf + offset, &FileSyncLog, sizeof(FILESYNC_LOGINFO));
    offset += sizeof(FILESYNC_LOGINFO);
    Send( SendBuf, head.TotalLength);
}
//
/*******************************************************************************
 * Function     : FileTask::UpdateResendFileList
 * Description  : 更新重发文件列表(内端使用)
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::UpdateResendFileList(FILE_SESSIONID sId)
{
	auto &resend = CurRecv.ResendList;
    if(find(resend.begin(),resend.end(),sId) != resend.end())
    	return;
    if(CurFile.Info.nSessionId <= 0)
    	return ;
    for(FILE_SESSIONID id = CurFile.Info.nSessionId; id <= sId; id++)
    {
        resend.push_back(id);
        LOG_WARN("id=%d, m_ResendFileList=%d",id,resend.size());
    }
}

/*******************************************************************************
 * Function     : CSslFileTask::UpdateRefuseFileList
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::UpdateRefuseFileList(FILE_SESSIONID sId)
{
	auto &refuse = CurRecv.RefuseList;
    auto it = find(refuse.begin(),refuse.end(),sId);
    if(it != refuse.end())
    	return;

    refuse.push_back(sId);
    LOG_WARN("sId=%d, m_RefuseFileList=%d",sId,refuse.size());
}
/*******************************************************************************
 * Function     : FileTask::OnRevResendFileReq
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::OnRevResendFileReq(char *buf, int nlen)
{
    if (NULL == buf)
        return;
    int taskID = 0;
    int RecvedNum = 0;
    int nResendNum = 0;
    int nRefuseNum = 0;
    int offset = 0;
    memcpy(&taskID, buf + offset, sizeof(taskID));
    offset += sizeof(taskID);

    memcpy(&RecvedNum, buf + offset, sizeof(RecvedNum));
    offset += sizeof(RecvedNum);
    this->TaskRespState = (RecvedNum < 0)?(2):(1);//

    memcpy(&nResendNum, buf + offset, sizeof(nResendNum));
    offset += sizeof(nResendNum);

    FILE_SESSIONID session;
    for (int i = 0; i < nResendNum; i++)
    {
        memcpy(&session, buf + offset, sizeof(session));
        offset += sizeof(session);
        CurRecv.ResendList.push_back(session);
    }

    memcpy(&nRefuseNum, buf + offset, sizeof(nRefuseNum));
    offset += sizeof(nRefuseNum);
    for (int i = 0; i < nRefuseNum; i++)
    {
        memcpy(&session, buf + offset, sizeof(session));
        offset += sizeof(session);
        CurRecv.RefuseList.push_back(session);
    }

	LOG_DEBUG("%d file is refused, %d file need to resend", nRefuseNum,nResendNum); 
	tsk_Cond.notify_all(); // 唤醒所有线程.
}

//文件命令处理
/*******************************************************************************
 * Function     : CSslAnalyse::FileProc
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void FileTask::FileProc(char *buf, int len_buf)
{
    GAPPACK_HEADER *cmd_Header = (GAPPACK_HEADER *)buf;
    char *pDataBuf = buf + sizeof(GAPPACK_HEADER);
    int datalen = cmd_Header->TotalLength - sizeof(GAPPACK_HEADER);
    switch (cmd_Header->CommandCode) {
    case CMD_FILE_SEND_TASK_REQ:
        CurRecv.Files = 0;
        OnRevFileSendTaskReq(pDataBuf,datalen);
        break;
    case CMD_FILE_SEND_TASK_RSP:            //接收任务请求
        OnRevFileSendTaskRsp(pDataBuf,datalen);
        break;
    case CMD_FILE_SEND_FILE_INFO_REQ://TID
        OnRevFileSendFileInfoReq(pDataBuf,datalen);
        break;
    case CMD_FILE_SEND_FILE_INFO_RSP:       //收到文件信息（长度、文件名等）
        OnRevFileSendFileInfoRsp(pDataBuf,datalen);
        break;
    case CMD_FILE_SEND_FILE_DATA://TID
        OnRevFileData(pDataBuf,datalen);
        break;
    case CMD_FILE_SEND_FILE_FINISHED_REQ://TID
        OnRevSendFileFinishedReq(pDataBuf,datalen);
        break;
    case CMD_FILE_FINISHED_TASK_REQ:         //任务完成消息包
        OnRevTaskFinishedReq(pDataBuf,datalen);
        break;
    case CMD_FILE_FINISHED_TASK_RSP:  //重发文件请求
        OnRevResendFileReq(pDataBuf,datalen);
        break;
    case CMD_FILE_TRANCEMODE_SET_REQ:
    	LOG_WATCH(CMD_FILE_TRANCEMODE_SET_REQ);
    	OnRecvTranceModeReq(pDataBuf,datalen);
    	break;
    default:
    	LOG_ERROR("收到未知文件命令: %d", cmd_Header->CommandCode);
        break;
    }
}
/*******************************************************************************
 * Function     : FileTask::Send
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::Send(const char *data, const size_t len,bool wait)
{
	this->Buffer.data.append(data,len);
	if(!wait || this->Buffer.isfull())
	{
		LOG_DEBUG("wait=%d,isfull=%d, this->Buffer.data.size()=%d"
			,wait,this->Buffer.isfull(),this->Buffer.data.size());
//		LOG_INFO("buffer is full > %ud",this->Buffer.MAX_LEN);
		g_Socket->WaitWrite(this->Buffer.data.data(), this->Buffer.data.size());
		this->Buffer.reset(); 
	}
	return 1;
}
/*******************************************************************************
 * Function     : FileTask::Send
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int FileTask::Send(const char *cmd, const size_t len)
{
	return g_Socket->WaitWrite(cmd, len);
}
/*******************************************************************************
 * Function     : FileTask::GetHeadPackValue
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
bool FileTask::GetHeadPackValue(unsigned char nCmdType, unsigned char nCmdCode, unsigned int nDataLength,
	        GAPPACK_HEADER &PackHead)
{
	return g_Socket->GetHeadPackValue(nCmdType, nCmdCode, nDataLength, PackHead);
}
