#include "../com/stdafx.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <signal.h>
using namespace std;
#ifdef OS_WIN
#include <direct.h>  
#include <io.h>
#endif

#include "../log/log.h"
#include "../Main/config.h"
#include "../com/os_app.h"
#include "../com/swgap_comm.h"
#include "../Socket/Socket.h"
#include "../Task/FileTask.h"
#include "Analyse.h"
#define  VERSION  "V1.0"

extern Socket *g_Socket;
extern std::mutex main_Mutex; // 全局互斥锁.
extern std::map<int, FileTask*> g_FileTask;
std::list<SAFECLIENT_TASKINFO> g_ClientTaskList;//客户端任务配置信息列表（由安全导入服务器下发）
/*******************************************************************************
 * Function     : Analyse::Start
 * Description  : create thread to analyse data
 * Input        :
 * Return       :Connect
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Analyse::Start(void)
{
	this->alyPthread = std::thread(&Analyse::OnReceive,this);
}

/********************************************************************************
 Function:       CSslSocketLayer
 Description:    与安全导入服务器通信类的构造函数
 Calls:
 Table Accessed:
 Table Updated:
 Input:
 Output:
 Return:
 Others:
 *********************************************************************************/
Analyse::Analyse(void)
{
	UpdateTask = -1;
    anlybuf = (char*) malloc(ANLY_BUF_LEN);
    anlybuflen = 0;					   //<当前数据缓冲区内的数据长度
    bCheckState = true;
    m_bLogin = false;                  //<是否已登录
    bThreadStart = false;
}
/********************************************************************************
 Function:       CSslSocketLayer
 Description:    与安全导入服务器通信类的构造函数
 Calls:
 Table Accessed:
 Table Updated:
 Input:
 Output:
 Return:
 Others:
 *********************************************************************************/
Analyse::Analyse(int size)
{
	UpdateTask = -1;
    anlybuf = (char*) malloc(size);
    anlybuflen = 0;					   //<当前数据缓冲区内的数据长度
    bCheckState = true;
    m_bLogin = false;                  //<是否已登录
    bThreadStart = false;
}

/*******************************************************************************
 * Function     : Analyse::~Analyse
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
Analyse::~Analyse(void)
{
	this->Release();
}
/*******************************************************************************
 * Function     : Analyse::Stop
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Analyse::Stop(void)
{
	bThreadStart = false;
}

/********************************************************************************
 Function:       Send
 Description:    发送数据封装函数EncryptProc
 Calls:
 Table Accessed:
 Table Updated:
 Input:          fd,socket句柄
 cmd,要发送的数据缓冲区
 len,要发送的数据长度
 Output:
 Return:         -1, 发送失败
 >=0,成功发送的字节数
 Others:
 *********************************************************************************/
int Analyse::Send( const char *buf, const size_t len)
{
    return g_Socket->WaitWrite(buf, len);
}

/********************************************************************************
 Function:       Release
 Description:    释放资源(socket、线程、内存等)
 Calls:
 Table Accessed:
 Table Updated:
 Input:
 Output:
 Return:
 Others:
 *********************************************************************************/
void Analyse::Release(void)
{
    m_bLogin = false;
    bThreadStart = false;
    if(alyPthread.joinable())
    {
    	LOG_INFO("alyPthread.join()");
    	alyPthread.join();
    }
}

#ifdef OS_LINUX
int timer_flag = 0;
void timer(int sig)
{
    if(SIGALRM == sig) 
    	timer_flag = 1;  
    return ;
}
#endif
/*******************************************************************************
 * Function     : Analyse::OnReceive
 * Description  : TODO
 * Input        :GetHeadPackValue
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
char *buffer = new char[RW_SOCKT_BUF_LEN_];
void Analyse::OnReceive(void)
{
	unsigned int len = ANLY_BUF_LEN;
	unsigned int tout_ms = 500;
#ifdef OS_LINUX
    signal(SIGALRM, timer);
    alarm(1);
#endif
	anlybuflen = 0;
	bThreadStart = true;
	int rlen = 0;
	while(bThreadStart)
	{ 
		if(g_Socket == NULL)
		{
			SLEEP_S(1);
			continue;
		}
#ifdef OS_LINUX
		if(timer_flag)
		{
			timer_flag = 0;
			this->CheckSocketState(SIGALRM);
			alarm(1);
		}
#endif
#ifdef OS_WIN
		this->CheckSocketState(0);
#endif
		LOG_TRACE("waiting to get data...");
	    rlen = std::min<int>(g_Socket->rFifo->Len(), (ANLY_BUF_LEN - anlybuflen));
	    memset(buffer, 0, len);
		if((len = g_Socket->rGet(buffer, rlen, 0, tout_ms)) <= 0)
			continue;
		if ((anlybuflen + len) > ANLY_BUF_LEN)   //缓冲区内数据太多，直接将这部分数据丢掉
		{
			LOG_ERROR("%d > RECBUF_LEN", (anlybuflen + len));
			len = ANLY_BUF_LEN - anlybuflen;
		}
	    memcpy(anlybuf + anlybuflen, buffer, len);
	    anlybuflen += len;
		if(anlybuflen >= sizeof(GAPPACK_HEADER))
		{
			this->AnalysePacket();
			SLEEP_US(10);
		}else{
			SLEEP_US(100);
		}
	}
}
/*******************************************************************************
 * Function     : Analyse::CheckSocketState
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Analyse::CheckSocketState(int sig)
{
#ifdef OS_LINUX
    if(SIGALRM != sig)return ;
#endif
    time_t tNow = time(NULL);
    if (((tNow - g_Socket->GetLastAlivedTime()) > 90) && this->bCheckState)
    {
    	LOG_WARN("Test time out, reconnect now...");
    	this->m_bLogin = false;
        g_Socket->ReConnect();
    }else if(!g_Socket->IsAlived())
    {
    	LOG_WARN("Socket is not alive!!!");
    	return ;
    }
    else if ((tNow - g_Socket->GetLastAlivedTime()) >= 30)
    {
    	LOG_DEBUG("Send heard test...");
    	this->SendTest();
    }
}
/********************************************************************************
 Function:       AnalyseReceiveData
 Description:    协议解析(对收到的数据进行协议解析)
 Calls:
 Table Accessed:
 Table Updated:
 Input:          buf   收到的数据缓冲区
 buflen  收到的数据长度
 Output:
 Return:
 Others:
 *********************************************************************************/
void Analyse::AnalysePacket(void)
{
    GAPPACK_HEADER *cmd_Header = NULL;
    unsigned int Offset = 0;				//记录tempbuf的偏移量

    while (1)
    {
        if (Offset >= anlybuflen)                             //数据已拷贝完成
        {
        	anlybuflen = 0;
        	return;
        }

        if((anlybuflen - Offset) < sizeof(GAPPACK_HEADER))
        	break;

        cmd_Header = (GAPPACK_HEADER*) (anlybuf + Offset);
        if ((strncmp(cmd_Header->HeadFlag, "####", 4) != 0 &&
        		strncmp(cmd_Header->HeadFlag, "@@@@", 4) != 0 )||
        		cmd_Header->Version != CMD_VERS_ONE)
        {
        	LOG_WARN("not find pack header");
            Offset++;
            continue;
        }
        assert(cmd_Header->TotalLength >= sizeof(GAPPACK_HEADER));
        if (cmd_Header->TotalLength > (anlybuflen - Offset))	//剩余数据包不全
            break;
        ProcCmd(anlybuf + Offset,cmd_Header->TotalLength);
        Offset += cmd_Header->TotalLength;
    }
    anlybuflen -= Offset;
    memmove(anlybuf, anlybuf + Offset, anlybuflen);
    return;
}

/********************************************************************************
 Function:       EncryptProc
 Description:    处理数据包类型
 Calls:
 Table Accessed:
 Table Updated:
 Input:          buf   收到的数据缓冲区
                 buflen  收到的数据长度
 Output:
 Return:
 Others:
 *********************************************************************************/
void Analyse::ProcCmd(char *buf, int len_buf)
{
	GAPPACK_HEADER *cmd_Header = (GAPPACK_HEADER *) buf; 
	switch (cmd_Header->CommandType)
	{
		case CMD_TYPE_ENC:
		{
			EncryptProc(buf,len_buf);
			break;
		}
		default:
		{
			BusProc(buf,len_buf);
			break;
		}
	}
}
/*******************************************************************************
 * Function     : EncryptProc
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Analyse::EncryptProc(char *buf, int len_buf)
{
	GAPPACK_HEADER *cmd_Header = (GAPPACK_HEADER *) buf;
	char *pDataBuf = buf + sizeof(GAPPACK_HEADER);

	switch (cmd_Header->CommandCode)
	{
	case CMD_ENC_SIGN:      // 服务端验证客户端

		break;
	case CMD_ENC_VERF:      // 客户端处理验证结果
		LOG_DEBUG("awnser from server %d(0:success)", (*(int * )pDataBuf));
		if (!(*(int *) pDataBuf))
		{ // success
			if (g_Socket->ukey_GetSymEncryKey())
			{
				LOG_ERROR("can not get session Secret key");
				exit(1);
			}
			char buf[512] = {0};
			int len = 0;
			g_Socket->ukey_GetSymKeyData(buf, len);
			Send(buf, len);//this->
		}
		else
		{
			exit(1); // 后期抛异常
		}
		break;
	case CMD_ENC_KEYSEND:   // 服务端接收会话密钥

		break;
	case CMD_ENC_KEYANS:    // 客户端获取回应
		LOG_INFO("answer %d", (*(int * )pDataBuf));
		if (!(*(int *) pDataBuf))
		{
			g_Socket->SetEncrypt(2);
			LOG_INFO("OnRevEncryptReq Encrypt=%d", g_Socket->Encrypt);
			SendLoginReq();
		}
		break;
	case CMD_ENC_DATA:
		int len;
		char *data = new char[cmd_Header->TotalLength - sizeof(GAPPACK_HEADER)];
		g_Socket->ukey_SymDecrypt((const char *) pDataBuf, cmd_Header->TotalLength - sizeof(GAPPACK_HEADER), data, len);
		BusProc(data, len);
		delete[] data;
		break;
	}
}
/*******************************************************************************
 * Function     : BusProc
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Analyse::BusProc(char *buf, int buflen)
{
	GAPPACK_HEADER *cmd_Header = (GAPPACK_HEADER *) buf;
	LOG_DEBUG("Header CommandType:%x,CommandCode:%x,TotalLength=%d",
			(cmd_Header)->CommandType,
			(cmd_Header)->CommandCode,
			(cmd_Header)->TotalLength);
    // 通信类型
    switch (cmd_Header->CommandType) {

    case CMD_TYPE_SYST:
        SystProc(buf, cmd_Header->TotalLength);
        break;
    case CMD_TYPE_TASK:
        TaskProc(buf, cmd_Header->TotalLength);
        break;
    case CMD_TYPE_FILE:
    	LOG_DEBUG("CMD_TYPE_FILE");
        FileProc(buf, cmd_Header->TotalLength);
        break;
    }
}
//任务处理
/*******************************************************************************
 * Function     : CSslAnalyse::TaskProc
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Analyse::TaskProc(char *buf, int len_buf)
{
    GAPPACK_HEADER *cmd_Header = (GAPPACK_HEADER *)buf;
    char *pDataBuf = buf + sizeof(GAPPACK_HEADER);
    switch (cmd_Header->CommandCode)
    {
    case CMD_TASK_SAFECLIENT_TASKINFO:  //客户端任务配置信息
        OnRevClientTaskInfo(pDataBuf,
                cmd_Header->TotalLength - sizeof(GAPPACK_HEADER));
        break;
    default:
        LOG_ERROR("收到未知任务命令: %d", cmd_Header->CommandCode);
        break;
    }
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
void Analyse::FileProc(char *buf, int buf_len)
{
	int taskId = 0;
	memcpy(&taskId, buf + sizeof(GAPPACK_HEADER), sizeof(taskId));
	if(g_FileTask.count(taskId) > 0)
	{
		g_FileTask[taskId]->FileProc(buf, buf_len);
	}
}
//系统处理
/*******************************************************************************
 * Function     : CSslAnalyse::SystProc
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Analyse::SystProc(char *buf, int len_buf)
{
    GAPPACK_HEADER *cmd_Header = (GAPPACK_HEADER *)buf;
    char *pDataBuf = buf + sizeof(GAPPACK_HEADER);
    int lenDataBuf = cmd_Header->TotalLength - sizeof(GAPPACK_HEADER);
    switch (cmd_Header->CommandCode) {
    case CMD_SYST_CONNECT: // 0x01  连接成功(client)
		LOG_INFO("OnRevConnect ++++++++++++");
		OnRevConnect(pDataBuf, lenDataBuf);
		break;

	case CMD_SYST_ENCRYPT_RSP:  // 0x08 响应加密模式请求
		OnRevEncryptFlagRsp(pDataBuf, lenDataBuf);
		break;

    case CMD_SYST_LOGIN: // 0x02  身份验证
        //LOG_ERROR("CMD_SYST_LOGIN");
        break;
    case CMD_SYST_LOGERR: // 0x03  登录失败
        //LOG_ERROR("CMD_SYST_LOGERR");
        OnRevLoginErr(pDataBuf, lenDataBuf);
        //将退出程序
        break;
    case CMD_SYST_WELCOME:              //   0x04  登录成功
        LOG_INFO("CMD_SYST_WELCOME"); 
        LOG_INFO("Login success!!!");
        OnRevLoginSucceed(pDataBuf, lenDataBuf);
        break;
    case CMD_SYST_TEST:                 //   0x05  连接测试
        LOG_INFO("OnRevTestRsp");
        break;
    default:
        LOG_ERROR("Unkonw cmd: %d", cmd_Header->CommandCode);
        break;
    }
}
/********************************************************************************
 Function:       OnRevConnect
 Description:    接收主模块发送连接成功消息包
 Calls:
 Table Accessed:
 Table Updated:
 Input:          nCmdType   命令类型
 nCmdCode   命令码
 nDataLength 该数据包的总长度
 Output:         PackHead 已经组好的包头信息
 Return:         true 成功 false 失败
 Others:
 *********************************************************************************/
void Analyse::OnRevConnect(char *buf, int nlen)
{
    if (NULL == buf)
        return;
    int nServerState = 0;
    memcpy(&nServerState, buf, 4);
    LOG_WATCH(nServerState);
    if (nServerState == 1)       //可以连接，发送用户登录信息
    {
    	g_Socket->SetEncrypt(0);       //原始会话不加密
        SendEncryptFlagReq(Config::Com.nEncrypt);
    }
}

/********************************************************************************
 Function:       SendEncryptFlagReq
 Description:    发送传输加密模式请求
 Calls:
 Table Accessed:
 Table Updated:
 Input:
 Output:
 Return:         成功发送的字节数
 Others:
 *********************************************************************************/
int Analyse::SendEncryptFlagReq(int encrypt)
{
    GAPPACK_HEADER head;
    char SendBuf[sizeof(GAPPACK_HEADER) + sizeof(encrypt)] = {0};
    int offset = 0;
    LOG_WATCH(encrypt);
    g_Socket->GetHeadPackValue(CMD_TYPE_SYST, CMD_SYST_ENCRYPT_REQ, sizeof(encrypt), head);
    memcpy(SendBuf + offset, &head, sizeof(GAPPACK_HEADER));
    offset += sizeof(GAPPACK_HEADER);
    memcpy(SendBuf + offset, &encrypt, sizeof(encrypt));
    offset += sizeof(encrypt);
    return Send(SendBuf, head.TotalLength);
}

/********************************************************************************
 Function:       SendLoginReq
 Description:    发送用户登录信息请求
 Calls:
 Table Accessed:
 Table Updated:
 Input:
 Output:
 Return:
 Others:
 *********************************************************************************/
int Analyse::SendLoginReq(void)
{
    GAPPACK_HEADER head;
    char SendBuf[256] = { 0 };
    char strUserName[32] = { 0 };
    char strPassword[32] = { 0 };
    char strVersion[32] = { 0 };
    int offset = 0;
    strncpy(strUserName, Config::Com.strUser,sizeof(strUserName));//"bdgap");
    strncpy(strPassword, Config::Com.strPass,sizeof(strPassword));//"bdgap3399");
    g_Socket->GetHeadPackValue(CMD_TYPE_SYST, CMD_SYST_LOGIN, sizeof(strUserName) + sizeof(strPassword) + sizeof(strVersion), head);
    memcpy(SendBuf + offset, &head, sizeof(head));
    offset += sizeof(head);
    memcpy(SendBuf + offset, strUserName, sizeof(strUserName));
    offset += sizeof(strUserName);
    memcpy(SendBuf + offset, strPassword, sizeof(strPassword));
    offset += sizeof(strPassword);
    strcpy(strVersion, (char*) VERSION);
    memcpy(SendBuf + offset, &strVersion[0], sizeof(strVersion));
    offset += sizeof(strVersion);
    return Send( SendBuf, head.TotalLength);
}

/********************************************************************************
 Function:       OnRevLoginErr
 Description:    连到登录消息错误处理
 Calls:
 Table Accessed:
 Table Updated:
 Input:          buf   收到的数据缓冲区
 nlen  收到的数据长度
 Output:
 Return:
 Others:
 *********************************************************************************/
void Analyse::OnRevLoginErr(char *buf, int nlen)
{
    if (NULL == buf)
        return;
    int nErrCode = 0;
    char strMsg[512] = { 0 };
    memcpy(&nErrCode, buf, 4);
    switch (nErrCode)
    {
        case 1:
            strcpy(strMsg, "Failure to login, user or passwd is wrong, please check!!");
            break;
        case 2:
            strcpy(strMsg, "Failure to login, user or passwd is wrong, please check!!");
            break;
        case 3:
            strcpy(strMsg, "Failure to login! do not find the solution in the database, check please!!");
            break;
        default:
            break;
    }
#ifdef OS_WIN
    char * str = UTF8ToGBK(strMsg);
    if(str)
    {
        LOG_INFO("%s",str);
        delete str;
    }
#endif

#ifdef OS_LINUX
    LOG_INFO("%s", strMsg);
#endif
    m_bLogin = false;
    bCheckState = false;   //设置为-1表示，不用再创建连接及叛断连接状态了
    g_Socket->Stop();
}

/********************************************************************************
 Function:       OnRevLoginSucceed
 Description:    收到登录成功消息包
 Calls:
 Table Accessed:
 Table Updated:
 Input:          buf   收到的数据缓冲区
 nlen  收到的数据长度
 Output:
 Return:
 Others:
 *********************************************************************************/
void Analyse::OnRevLoginSucceed(char *buf, int nlen)
{
    memcpy(&Config::SvrInfo, buf, sizeof(GAPSERVER_INFO));
    LOG_INFO("OnRevLoginSucceed workmode=%d outip=%s outPort=%d innerIp=%s innerPort=%d", Config::SvrInfo.nWorkMode,
    		Config::SvrInfo.strOutsideServerIP, Config::SvrInfo.nOutSidePort, Config::SvrInfo.strInnerServerIP,
			Config::SvrInfo.nInnerPort);
    m_bLogin = true;
}

/********************************************************************************
 Function:       SendTest
 Description:    发送测试（心跳）包
 Calls:
 Table Accessed:
 Table Updated:
 Input:
 Output:
 Return:
 Others:
 *********************************************************************************/
int Analyse::SendTest()
{
    if (!m_bLogin)           //登录不成功则不发送心跳包
        return -1;
    GAPPACK_HEADER head;
    char SendBuf[sizeof(GAPPACK_HEADER) + 1];
    g_Socket->GetHeadPackValue(CMD_TYPE_SYST, CMD_SYST_TEST, 0, head);
    memcpy(SendBuf, &head, sizeof(GAPPACK_HEADER));
    return Send( SendBuf, head.TotalLength);
}

/********************************************************************************
 Function:       OnRevEncryptFlagRsp
 Description:    收到加密标志消息包
 Calls:
 Table Accessed:
 Table Updated:
 Input:          buf   收到的数据缓冲区
 nlen  收到的数据长度
 Output:
 Return:
 Others:
 ********************************************************************************/
void Analyse::OnRevEncryptFlagRsp(char *buf, int nlen)
{
    if (NULL == buf)
        return;
    //加密方式保持与服务端一致
    int encrypt = 0;
    memcpy(&encrypt, buf, sizeof(int));
    if(encrypt == 0 || encrypt == 1)
    {
    	g_Socket->SetEncrypt(encrypt);//set encrypt flag here
        LOG_INFO("OnRevEncryptFlag Encrypt=%d ", g_Socket->Encrypt);
    	SendLoginReq();
    }
    else if(encrypt == 2)
    {
		char sndbuf[128]={0};
		if(g_Socket->ukey==NULL)
		{
			g_Socket->ukey = new UKey();
		}
		if(g_Socket->ukey != NULL)
		{
			g_Socket->ukey_GetSignData(sndbuf, nlen);
			Send(sndbuf, nlen);
			LOG_INFO("send the sign packet");
		}
    }
}

/********************************************************************************
 Function:       OnRevClientTaskInfo
 Description:    接收到任务信息
 Calls:
 Table Accessed:
 Table Updated:
 Input:          buf   收到的数据缓冲区
 nlen  收到的数据长度
 Output:
 Return:
 Others:
 *********************************************************************************/
void Analyse::OnRevClientTaskInfo(char *buf, int nlen)
{
    if (NULL == buf)
        return;
    int nTatalTask = 0;
    int nIndex = 0;
    SAFECLIENT_TASKINFO taskinfo = { 0 };
    memcpy(&nTatalTask, buf, sizeof(int));
    memcpy(&nIndex, buf + sizeof(int), sizeof(int));
    memcpy(&taskinfo, buf + sizeof(int) * 2, sizeof(SAFECLIENT_TASKINFO));

    std::unique_lock <std::mutex> lck(main_Mutex);
    if (nIndex == 1)  //第一个任务信息
    {
        UpdateTask = 1; 
        g_ClientTaskList.clear(); 
    }
    g_ClientTaskList.push_back(taskinfo); 
    if (nTatalTask == 0)
    {
        LOG_INFO("TaskNum=0,Task list is clear");
        return;
    }
    LOG_INFO("\nTaskNum=%d,index=%d,"
    		"TaskID=%d, taskName=%s, srcPath=%s, "
    		"transmod=%d, limitspeed=%d, nStableTime=%d, "
    		"filecheck=%d, status=%d\n",
    		nTatalTask, nIndex,
			taskinfo.rule.nTaskID,taskinfo.rule.strTaskName, taskinfo.rule.strSrcPath,
			taskinfo.rule.nTransmod,taskinfo.rule.nLimitSpeed, taskinfo.rule.nSyncTime,
			taskinfo.rule.nLinkFileCheckId, taskinfo.rule.status);
	 
    if (nIndex == nTatalTask)
    {
        UpdateTask = 2;
		
		char fname[50] = "bdSWGapTask.tsk";
		string  strTaskFile = Config::GetExePath() + os_format2path(fname,sizeof(fname));

		FILE *pFile = fopen(strTaskFile.c_str(), "wb");
		if (pFile == NULL)
		{
			LOG_WARN("Open file failed %s", strTaskFile.c_str());
			return;
		} 
		char strLine[512];
		for (auto it : g_ClientTaskList)
		{
			memset(strLine, 0, sizeof(strLine));
			sprintf(strLine, "task=%d;%d;%d;%s;%s;%d\r\n",
            		it.rule.nTaskID, it.rule.nSecretLevel,
					it.rule.nSyncTime, it.rule.strSrcPath,
					it.rule.strTaskName, it.rule.status);
			fwrite(strLine, strlen(strLine), 1, pFile);
		}
		fclose(pFile);
    }
}

