/*
 * SFileTask.cpp
 *
 *  Created on: 2018年9月7日
 *      Author: luzxiang
 */

#include <sys/types.h>
#include <sys/stat.h>
#ifdef OS_LINUX
#include <dirent.h>
#include <unistd.h>
#endif
#include <fcntl.h>
#include <algorithm>
#include <string.h>
#include <string>

#include "../com/stdafx.h"
#include "../log/log.h"
#include "../Main/config.h"
#include "../Analyse/Analyse.h"
#include "../Socket/Socket.h"
#include "../com/swgap_comm.h"
#include "SFileTask.h"
#include "../com/os_app.h"

extern Analyse *g_AnalyseLayer;

/*******************************************************************************
 * Function     : SFileTask::SFileTask
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
SFileTask::SFileTask(FILESYNC_RULE &r):FileTask(r)
{
	if(Config::Com.nMaster)
	{
    	LOG_DEBUG("set the trance mode to send");
    	while(this->SendTranceModeReq(0, 1) <= 0)
    	{
    		SLEEP_S(3);
        	LOG_DEBUG("set the trance mode to send");
    	}
		this->transMod = SWGAP_SND;
	}
	else
	{
    	LOG_DEBUG("set the trance mode to recv");
    	while(this->SendTranceModeReq(0, 0) <= 0)
    	{
    		SLEEP_S(3);
        	LOG_DEBUG("set the trance mode to recv");
    	}
		this->transMod = SWGAP_RCV;
	}
	LOG_DEBUG("双向任务创建成功");
}
/*******************************************************************************
 * Function     : SFileTask::~SFileTask
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
SFileTask::~SFileTask(void)
{

}

/*******************************************************************************
 * Function     : FileTask::ScanSrcPath
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int SFileTask::ScanPathFileTo(char *srcPath, FileList_St& fList){
	os_format2dir(srcPath,1024);
	LOG_WATCH(srcPath);
	int workPathLen = strlen(fList.workPath);
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
	while ((s_dir = readdir(dir)) != NULL)
	{
		if ((strcmp(s_dir->d_name, ".") == 0) || (strcmp(s_dir->d_name, "..") == 0))
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
			this->MoveFileToTemp();
			SLEEP_S(5);
		}
		OS_SNPRINTF(curdir, sizeof(curdir), "%s%s", srcPath, s_dir->d_name);
//		os_topath(curdir, sizeof(curdir));
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

	if(szDir.Right(1) != "\\")
	szDir += "\\";
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
			this->MoveFileToTemp(); 
			SLEEP_S(5);
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
 * Function     : SFileTask::MoveFileToTemp
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void SFileTask::MoveFileToTemp(void)
{
	LOG_DEBUG("正在加载扫描到的%d个文件",this->TempPath.List.size());
    std::unique_lock <std::mutex> lck(tsk_Mutex);
	while(this->TempPath.List.size() >= this->rule.nLimitSpeed * 3)
	{
		lck.unlock();
		SLEEP_MS(100);
		lck.lock();
	}
	for (auto file : this->TaskPath.List)
	{
		this->TempPath.List.push_back(file);
		this->TempPath.FileSize += file.nFileSize;
	}
	this->TempPath.Flag = 2;
//	lck.unlock();
	LOG_DEBUG("文件加载完毕,还有%d个文件在等待发送",this->TempPath.List.size());
	this->TaskPath.Reset();
}

/*******************************************************************************
 * Function     : SFileTask::ScanAndMoveFile
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void SFileTask::ScanAndMoveFile(void){
	if (SWGAP_SND != this->transMod)
	{
		LOG_WATCH(this->transMod);
		return;
	}
	if (this->rule.status == 0)            //
	{
		return;            //if this task is not enable, continue next task;
	}
	if ((time(NULL) - this->tLastSync) < Config::Sender.nWait)
	{
		return;
	}
	this->tLastSync = time( NULL);

	if (0 == this->TaskPath.Flag)
	{
		this->TaskPath.FileSize = 0;
		LOG_DEBUG("正在扫描目录%s",this->TaskPath.workPath);
		this->ScanPathFileTo(this->TaskPath.workPath, this->TaskPath);
		LOG_DEBUG("目录扫描完成,%d个文件等待处理",this->TaskPath.List.size());
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
	if (0 == this->TaskPath.Flag){
		this->transMod = SWGAP_RS;
	}
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
void SFileTask::OnRevFileSendTaskRsp(char *buf, int nlen)
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
    FILE_SESSIONID sid;

    this->CurRecv.RefuseList.clear();
    TaskRespState = nDiskFlag;

    if(TaskRespState == 1)
	for(int i = 0; i < size; ++i)
	{
		memcpy(&sid, buf + offset, sizeof(sid));
		offset += sizeof(sid);
//TODO remove the file from SendBuf
		this->CurRecv.RefuseList.push_back(sid);
	}
	LOG_DEBUG("收到请求应答，%d个文件不需要同步",this->CurRecv.RefuseList.size());
	tsk_Cond.notify_all(); // 唤醒所有线程.
}
/*******************************************************************************
 * Function     : SFileTask::GetSendFileTaskRsp
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int SFileTask::GetSendFileTaskRsp(bool bwait)
{
	if (0 == this->TaskRespState)
	{
		if (bwait)
		{
			std::unique_lock <std::mutex> lck(tsk_Mutex);
			tsk_Cond.wait_for(lck, std::chrono::seconds(30));
			LOG_DEBUG("唤醒后:this->m_TaskRespState = %d\n",
					this->TaskRespState);
		}
		else
		{
			LOG_WARN("Req task timeout!");
			return -1;
		}
	}else if (1 == this->TaskRespState){
		/* analyse the data that need to send */
		for (auto sid : this->CurRecv.RefuseList)
		{
    		this->SendBuf.List.remove_if([&](FILEINFOPACK_HEAD &finfo){
    			if(finfo.nSessionId != sid)
        			return false;
				this->SendBuf.FileSize -= finfo.nFileSize;
				return true;
    		});
		}
		LOG_DEBUG("已放弃不需要同步的%d个文件，剩下%d文件等待同步",
				this->CurRecv.RefuseList.size(),
				this->SendBuf.List.size());
		this->CurRecv.RefuseList.clear();
		this->TaskRespState = 0;
		return 0;
	}else {
		this->SendBuf.List.clear();
		this->SendBuf.FileSize = 0;
		LOG_DEBUG("已放弃文件同步,disk full!!!!!!!!");
		this->CurRecv.RefuseList.clear();
		this->TaskRespState = 0;
		return 0;
	}
	return GetSendFileTaskRsp(false);
}

/*******************************************************************************
 * Function     : FileTask::Send1TaskFile
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void SFileTask::SendFile(void){
	 
    if (this->rule.status == 0)
    {
		return;
    }
    if(this->transMod == SWGAP_RCV)
    {
        LOG_TRACE("transMod:%d ", this->transMod);
		return;
    }
    this->SendTaskFile();
    if(SWGAP_RS == this->transMod)
    {
		LOG_WATCH(this->TaskPath.List.size());
		LOG_WATCH(this->TempPath.List.size());
		LOG_WATCH(this->SendBuf.List.size());
		if(this->TempPath.List.size() > 0)
			this->TempPath.Flag = 2;
        if(this->TaskPath.List.empty() && this->TempPath.List.empty() && this->SendBuf.List.empty())
        {
			SLEEP_S(2);
        	if(this->SendTranceModeReq(0, 0) > 0)
        	{
            	LOG_DEBUG("Change the trance mode, from send to recv");
        		this->transMod = SWGAP_RCV;
        	}
        }
    }
}
/*******************************************************************************
 * Function     : SFileTask::WaitResendFailFile
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int SFileTask::WaitResendFailFile(bool bwait)
{
	LOG_WATCH(this->TaskRespState);
	if (0 == this->TaskRespState)
	{
		if (bwait)
		{
			this->CurRecv.ResendList.clear();
			this->CurRecv.RefuseList.clear();
			if (0 > this->SendTaskFinished(0))
			{
				LOG_WARN("send task finish failed!");
				return -1;    //等待重发
			}
			std::unique_lock <std::mutex> lck(tsk_Mutex);
			tsk_Cond.wait_for(lck, std::chrono::seconds(30));
			LOG_DEBUG("唤醒后:this->m_TaskRespState = %d\n",
					this->TaskRespState);
		}
		else
		{
			LOG_WARN("服务端没有返回,继续等待服务端返回,不发送TaskFinished 2!");
			return -1;
		}
	}
	else if(this->TaskRespState == 1)
	{
		this->SendBuf.FileSize = 0;
		auto &reSendFile = this->CurRecv.ResendList;
		auto &reFuseFile = this->CurRecv.RefuseList;
		std::vector<FILE_SESSIONID>::iterator itRSFile;
		char loginfo[500] = {0};

		auto iter = this->SendBuf.List.begin();
		for (; iter != this->SendBuf.List.end();)
		{
			itRSFile = find(reFuseFile.begin(),reFuseFile.end(),iter->nSessionId);
			string fname(this->TempPath.workPath);
//			fname += os_topath(iter->strFileName,sizeof(iter->strFileName)) ;
//			fname += "/" ;
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
//			else if (0 != remove(fname.c_str()))//删除文件失败
//			{
//				LOG_ERROR("del errorno:%d",errno,strerror(errno));
//			}
//			LOG_WATCH(fname.c_str());
			this->SendBuf.List.erase(iter++);
		}
		reSendFile.clear();
		reFuseFile.clear();
		this->TaskRespState = 0;
		return 0;
	}else {
		this->TaskRespState = 0;
		return 0;
	}
	return WaitResendFailFile(false);
}
/*******************************************************************************
 * Function     : SFileTask::HandTmpFile
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
bool SFileTask::HandTmpFile(void)
{
	//do nothing here
	return true;
}
/*******************************************************************************
 * Function     : SFileTask::SendFileTaskReq
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int SFileTask::SendFileTaskReq(int nLevel, int nNeedSpace)
{
    GAPPACK_HEADER head;
    int offset = 0;
    int size = this->SendBuf.List.size();
    int datasize = sizeof(nTaskID) + sizeof(nLevel) + sizeof(nNeedSpace)
    		+ sizeof(size) + size * sizeof(FILEINFOPACK_HEAD);
    char *SendBuf = new char[sizeof(GAPPACK_HEADER) + datasize];

    this->GetHeadPackValue(CMD_TYPE_FILE, CMD_FILE_SEND_TASK_REQ, datasize, head);
    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);

    memcpy(SendBuf + offset, &this->nTaskID, sizeof(this->nTaskID));
    offset += sizeof(this->nTaskID);
    memcpy(SendBuf + offset, &nLevel, sizeof(nLevel));
    offset += sizeof(nLevel);
    memcpy(SendBuf + offset, &nNeedSpace, sizeof(nNeedSpace));
    offset += sizeof(nNeedSpace);
    memcpy(SendBuf + offset, &size, sizeof(size));
    offset += sizeof(size);

    /* file info list */
    int id = 0;
    for(auto &finfo : this->SendBuf.List)
    {
    	finfo.nSessionId = id++;
		memcpy(SendBuf + offset, &finfo, sizeof(finfo));
		offset += sizeof(finfo);
    }
    int slen = this->Send( SendBuf, head.TotalLength);
    delete[] SendBuf;
    return slen;
}

/*******************************************************************************
 * Function     : SFileTask::SendFileTaskReq
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int SFileTask::SendTranceModeReq(int side, int mode)
{
    GAPPACK_HEADER head;
    int offset = 0;
    const int dsize = sizeof(GAPPACK_HEADER) + sizeof(nTaskID) + sizeof(side) + sizeof(mode);
    char SendBuf[dsize] = {0};

    this->GetHeadPackValue(CMD_TYPE_FILE, CMD_FILE_TRANCEMODE_SET_REQ, dsize, head);
    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);

    LOG_WATCH(this->nTaskID);
    memcpy(SendBuf + offset, &this->nTaskID, sizeof(this->nTaskID));
    offset += sizeof(this->nTaskID);
    memcpy(SendBuf + offset, &side, sizeof(side));
    offset += sizeof(side);
    memcpy(SendBuf + offset, &mode, sizeof(mode));
    offset += sizeof(mode);
    return this->Send( SendBuf, head.TotalLength);
}

/*******************************************************************************
 * Function     : SFileTask::SendFileTaskReq
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void SFileTask::OnRecvTranceModeReq(char *buf, int nlen)
{
    if (NULL == buf)
        return;
    int offset = 0;
    int taskID = 0;
    int side = 0;
    int mode = 0;

    memcpy(&taskID, buf + offset, sizeof(taskID));
    offset += sizeof(taskID);
    memcpy(&side, buf + offset, sizeof(side));
    offset += sizeof(side);
    memcpy(&mode, buf + offset, sizeof(mode));
    offset += sizeof(mode);
	this->transMod = mode == 0 ? SWGAP_SND : SWGAP_RCV;
	LOG_DEBUG("Change the transmod to %d",this->transMod);
}
/*******************************************************************************
 * Function     : FileTask::CheckFileInfo
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
bool SFileTask::CheckFileInfo(FILEINFOPACK_HEAD& finfo)
{
	bool rst = true;
	string fname(this->rule.strSrcPath); 
	fname += "/" ;
	fname += finfo.strFileName ;
	struct stat file_stat;

	if(os_access_fok(fname.c_str(),0) == 0)
	{
		//如果文件已经存在，且当前是主机，则不接收
		if(Config::Com.nMaster)
			return false;
#ifdef OS_WIN
		time_t tNow = time(NULL);
		struct _stat64 file_stat;              //支持大文件 大于3G的
		_stat64(fname.c_str(), &file_stat); 
#else
		stat(fname.c_str(), &file_stat);
#endif
		/*
			如果文件已经存在，且当前是备机,需要判断以下条件
		*/
		//备机文件修改时间比主机最后修改时间更近于当前时间，则不接收，否则以主机为准，接收文件
		if(finfo.mtime >= time(NULL) - file_stat.st_mtime)
			rst = false; 
	}
	return rst;
}
/*******************************************************************************
 * Function     : FileTask::OnRevFileSendTaskReq
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void SFileTask::OnRevFileSendTaskReq(char *buf, int nlen)
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
	CurRecv.reset();
	if(DiskIsOK == 1)
	{
		FILEINFOPACK_HEAD finfo;
		for(int i = 0; i < size; ++i)
		{
			memcpy(&finfo, buf + offset, sizeof(finfo));
			offset += sizeof(finfo);
			if(!CheckFileInfo(finfo))
			{
				CurRecv.RefuseList.push_back(finfo.nSessionId);
			}
		}
		if (rcvfsize >= 1024)
		{
			LOG_INFO("RevDownload Req TaskID =%d  level=%d   nTotalFileSize=%dMB",  taskID , nLevel , rcvfsize / (1024));
		}
		else
		{
			LOG_INFO("RevDownload Req TaskID =%d  level=%d   nTotalFileSize=%dKB",  taskID , nLevel , rcvfsize);
		}
	}

    GAPPACK_HEADER head;
    const int datasize = sizeof(taskID) + sizeof(DiskIsOK) + sizeof(size)+sizeof(FILE_SESSIONID)*size;
    char *SendBuf = new char[sizeof(GAPPACK_HEADER) + datasize];
    offset = 0;
    GetHeadPackValue(CMD_TYPE_FILE, CMD_FILE_SEND_TASK_RSP, datasize, head);
    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);
    memcpy(SendBuf + offset, &taskID, sizeof(taskID));
    offset += sizeof(taskID);
    memcpy(SendBuf + offset, &DiskIsOK, sizeof(DiskIsOK));
    offset += sizeof(DiskIsOK);

    size = CurRecv.RefuseList.size();
    memcpy(SendBuf + offset, &size, sizeof(size));
    offset += sizeof(size);
    for_each(CurRecv.RefuseList.begin(), CurRecv.RefuseList.end(),[&](FILE_SESSIONID id){
        memcpy(SendBuf + offset, &(id), sizeof(id));
        offset += sizeof(id);
    });
    Send(SendBuf, head.TotalLength);
	delete[] SendBuf;
    CurRecv.ResendList.clear();
    CurRecv.RefuseList.clear();
}
