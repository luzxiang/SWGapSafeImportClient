#ifndef __ANALYSE_H
#define __ANALYSE_H
#include "../com/stdafx.h"
#ifdef OS_LINUX
#include <signal.h>
#include <resolv.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include  <list>
#include  <vector>
#include "../com/swgap_comm.h"
using namespace std;

#define ANLY_BUF_LEN	  (RW_SOCKT_BUF_LEN_)	//解析缓冲区长度
extern std::list<SAFECLIENT_TASKINFO> g_ClientTaskList;

class Analyse
{
public:
	//与安全导入服务器通信类的构造函数
	Analyse();
	Analyse(int);
    //与安全导入服务器通信类的析构函数
    virtual ~Analyse(void);
    //释放资源(socket、线程、内存等)
    void Release(void);

    void Start(void);
    //发送数据封装函数
    int Send(const char *cmd, const size_t len);
    //模块接收到socket数据处理
    void OnReceive(void);
    //协议解析(对收到的数据进行协议解析)
    void AnalysePacket(void);

    void ProcCmd(char *buf, int len_buf);

    void EncryptProc(char *buf, int len_buf);

    void BusProc(char *buf, int buflen);

    void TaskProc(char *buf, int len_buf);

    void FileProc(char *buf, int len_buf);

    void SystProc(char *buf, int len_buf);

    //发送用户登录信息请求
    int SendLoginReq(void);
    //连到登录消息错误处理
    void OnRevLoginErr(char *buf, int nlen);
    //收到登录成功消息包
    void OnRevLoginSucceed(char *buf, int nlen);
    //发送测试（心跳）包
    int SendTest(void);
    //收到加密标志消息包
    void OnRevEncryptFlagRsp(char *buf, int nlen);
    //接收主模块发送连接成功消息包
    void OnRevConnect(char *buf, int nlen);
    //发送传输加密模式请求
    int SendEncryptFlagReq(int encrypt);
    //接收到任务信息
    void OnRevClientTaskInfo(char *buf, int nlen);

    void CheckSocketState(int sig);

    void Stop(void);

    std::thread alyPthread; 
    bool bThreadStart;
public:
    char *anlybuf;				//<Socket接收数据缓冲区
    unsigned int anlybuflen;	//<当前数据缓冲区内的数据长度
    bool bCheckState;	//<最后一次接收到数据的时间
    bool m_bLogin;		//<是否已登录
	/*0,未收到任务信息 1,正在更新任务信息中 2,任务信息更新已完成(需要同步) 3, 任务信息已同步*/
	int UpdateTask;
private:
};

#endif // CSSLSOCKETLAYER_H
