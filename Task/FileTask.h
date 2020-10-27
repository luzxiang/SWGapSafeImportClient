/*
 * FileTask.h
 *
 *  Created on: 2018年9月6日
 *      Author: luzxiang
 */

#ifndef FILETASK_H_
#define FILETASK_H_

#include <stdio.h>
#include <map>
#include <list>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;
#include "../com/swgap_comm.h"
#include "../Socket/Socket.h"
#define SLEEP_FOR 	std::this_thread::sleep_for
#define SLEEP_NS(x) SLEEP_FOR(std::chrono::nanoseconds((int64_t)(x)))//ns
#define SLEEP_US(x) SLEEP_FOR(std::chrono::microseconds((int64_t)(x)))//us
#define SLEEP_MS(x) SLEEP_FOR(std::chrono::milliseconds((int64_t)(x)))//ms
#define SLEEP_S(x) 	SLEEP_FOR(std::chrono::seconds((int64_t)(x)))//seconds

extern std::mutex filetask_Mutex; // 全局互斥锁.
extern std::condition_variable filetask_Cond; // 全局条件变量.
typedef unsigned int FILE_SESSIONID;

typedef struct FileList
{
	std::list<FILEINFOPACK_HEAD> List;  //当前已扫描到可以处理的文件队列
	unsigned long int FileSize;      	//当前已扫描到可以处理的文件大小总量
	unsigned int Flag;          	//处理标志 0不需要处理 1正在处理 2等待处理
	char workPath[PATH_LEN];		//work path of cur map

	FileList(void)
	{
		Reset();
		memset(workPath,0,sizeof(workPath));
	}
	void Reset(void)
	{
		printf("reset list...\n");
		List.clear();
		FileSize = 0;
		Flag = 0;
	}
}FileList_St;

int system_move(const char *oldname, const char* newname);

class FileTask{
public:
	unsigned int nTaskID;
	unsigned int tLastSync;            		 //最后一次同步时间

    struct {
	    unsigned int Files;       		//收到的文件数量（用于单批次任务）
		unsigned long int TotalSize;	//当前执行任务总发送字节数
	    std::vector<FILE_SESSIONID> ResendList;       //重发文件列表
	    std::vector<FILE_SESSIONID> RefuseList;       //拒绝文件列表
    	void reset(void)
    	{
    		Files = 0;
    		TotalSize = 0;
    		ResendList.clear();
    		RefuseList.clear();
    	}
    }CurSend,CurRecv;

    struct {
    	FILE *pHandle;				//当前打开的文件
    	FILEINFOPACK_HEAD Info;		//当前的文件信息
    	unsigned long int RecvSize;	//当前执行任务收到文件总字节数
    	void reset(void)
    	{
    		memset(&Info,0,sizeof(Info));	
    		if(pHandle)
    			fclose(pHandle);
    		pHandle = NULL;
    		RecvSize = 0;
    	}
    }CurFile;

    struct buf{
        string data;
        unsigned int MAX_LEN;
        void reset(void){
        	data.clear();
        	std::string().swap(data);
        }
		buf()
		{
        	data.clear();
        	std::string().swap(data);
			MAX_LEN = 4*1024*1024U;
		}
        bool isfull()
        {
        	return (data.length() >= MAX_LEN);
        }
    }Buffer;

	bool IsEnable;					//在任务启动之前，需要先处理任务缓存目录下的文件
	unsigned int stableTime;
	unsigned long int ScanLimitSize;
	TRANSMOD transMod;
    int FileRespState; 		//发送文件响应状态   0,无响应，1有响应(成功) ，2有响应(失败)
    int TaskRespState; 		//任务响应状态      0,无响应，1有响应(成功) ，2有响应(失败)

    std::mutex tsk_Mutex; 	// 全局互斥锁.
    std::condition_variable tsk_Cond; // 全局条件变量.

	FILESYNC_RULE rule;
	FileList_St TaskPath;		//存放扫描到源目录下的文件列表
	FileList_St TempPath;		//存放从源目录挪过来的文件列表
	FileList_St SendBuf;		//存放准备发送的文件列表

    int (*move_fun)(const char *, const char *);

	bool InitTask(void);

	TRANSMOD GetTransMod(TRANSDIRT transmod, int gapworkmod);

	virtual int ScanPathFileTo(char *srcPath, FileList_St& fList);

	bool Rename(const char *oldname, const char *newname);

	virtual void MoveFileToTemp(void);

	int SendTempFileFromBuf(void);

	virtual int GetSendFileTaskRsp(bool bwait);

	int SendFileData(FILEINFOPACK_HEAD &fileinfo, int sid);

	virtual int WaitResendFailFile(bool bwait);
//public:
	FileTask(FILESYNC_RULE &r);

	virtual ~FileTask(void);

	void Release(void);

	virtual bool HandTmpFile(void);

	virtual void ScanAndMoveFile(void);

	virtual void SendFile(void);

	void SendTaskFile(void);
	//发送任务请示包
	virtual int SendFileTaskReq(int nLevel, int nNeedSpace);
	//发送文件基本信息
	int SendFileInfo(FILEINFOPACK_HEAD fileInfo);
	//发送文件数据
	int SendFileData(int nSessionID, char *buf, int nDataLen);
	//发送文件发送完成包(在每个文件数据发送完成后发送该包)
	int SendFileFinished(int nSessionID, int nFlag);
	//向服务器发送任务已完成消息通知(外端)
	int SendTaskFinished(int nResult);
    //	收到服务器发送过来的推送任务请求包
	virtual void OnRevFileSendTaskReq(char *buf, int nlen);
    //
	virtual void OnRevFileSendTaskRsp(char *buf, int nlen);
    //	接收文件信息发送请求（外端）
    void OnRevFileSendFileInfoReq(char *buf, int nlen);
    //	接收文件发送信息响应包（外端）
    void OnRevFileSendFileInfoRsp(char *buf, int nlen);
    //	接收文件数据（内端）
    void OnRevFileData(char *buf, int nlen);
    //	接收到文件发送完成包（请求文件验证）
    int OnRevSendFileFinishedReq(char *buf, int nlen);
    //	接收任务完成消息包(内端)
    void OnRevTaskFinishedReq(char *buf, int nlen);
    //	接收到重发文件请求消息包(外端)
    void OnRevResendFileReq(char *buf, int nlen);

    void SendFileSyncLog(char*dir, FILEINFOPACK_HEAD &synFile, int status, char*logInfo);

    void SendFileAlarmcLog(FILEINFOPACK_HEAD &synFile,int level, int type,  char*logInfo);
    //	更新重发文件列表
    void UpdateResendFileList(FILE_SESSIONID nSessionId);
    //	更新重发文件列表
    void UpdateRefuseFileList(FILE_SESSIONID nSessionId);

    void FileProc(char *buf, int len_buf);

	int Send(const char *cmd, const size_t len, bool wait);
	int Send(const char *cmd, const size_t len);

	bool GetHeadPackValue(unsigned char nCmdType, unsigned char nCmdCode, unsigned int nDataLength,
	        GAPPACK_HEADER &PackHead);

	virtual void OnRecvTranceModeReq(char *buf, int nlen){

	}
};


#endif /* FILETASK_H_ */
