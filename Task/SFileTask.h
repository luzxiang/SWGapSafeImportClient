/*
 * SFileTask.h
 *
 *  Created on: 2018年9月7日
 *      Author: luzxiang
 */

#ifndef SFILETASK_H_
#define SFILETASK_H_
#include "FileTask.h"

class SFileTask : public FileTask{
public:
	SFileTask(FILESYNC_RULE &r);
	virtual ~SFileTask(void);
	int ScanPathFileTo(char *srcPath, FileList_St& fList);
	void MoveFileToTemp(void);

	int GetSendFileTaskRsp(bool bwait);
	void OnRevFileSendTaskRsp(char *buf, int nlen);
	void SendFile(void);
	int WaitResendFailFile( bool bwait);
	void ScanAndMoveFile(void);
	bool HandTmpFile(void);
	void OnRevFileSendTaskReq(char *buf, int nlen);
	bool CheckFileInfo(FILEINFOPACK_HEAD& finfo);

	int SendFileTaskReq(int nLevel, int nNeedSpace);
	int SendTranceModeReq(int side, int mode);
	void OnRecvTranceModeReq(char *buf, int nlen);
};



#endif /* SFILETASK_H_ */
