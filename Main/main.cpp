#include <string.h>
#include <string>
#include <map>
#include <list>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <mutex> 
using namespace std;
#include "../com/stdafx.h"
#include "../log/log.h"
#include "config.h"
#include "../Filemark/KAES.h"
#include "../Filemark/filemark.h"
#include "../Analyse/Analyse.h"
#include "../Socket/Socket.h"
#include "../Task/FileTask.h"
#include "../Task/SFileTask.h"

#ifdef OS_LINUX
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#ifdef OS_WIN 
#include <io.h>
extern "C"
{
#include "openssl/applink.c"
};
#endif

Socket *g_Socket = NULL;
Analyse *g_AnalyseLayer = NULL;  //与管理机通信的网络套接字对象指针
std::map<int, FileTask*> g_FileTask;

std::thread SendThread;//发送数据线程
std::mutex main_Mutex; // 全局互斥锁.
bool SendThread_Stop = true;
bool bMainStop = false;

/*  //////////////////////////////////////////////////////
 文件获取流程步聚：
 *1.遍历目录下所有文件，将符合文件（稳定时间大于3秒, 判断依据是本地时间与文件最后修改时间相比较），迁移至临时目录中,
 *其目录及文件结构保持与源目录一致主。当源目录下的子目录为空时，删除该子目录。
 *2.遍历临时目录，将所有文件上传至单导外端服务器，上传成功后，将整个临时文件迁移至备份目录中（备份目录为监时目录+时戳）。
 *3.上传过程中将记录日志（上传文件、告警、系统错误）。
 * 重复以上操作
 *   ////////////////////////////////////////////////////////////
 */

//发送文件支持断点续传流程:
//1.与对端建立TCP连后，由发送端向接收端发送任务推送请求，接收到将根据情况发送，重发未成功发送重发的文件(任务)请求。推送端再重发未发送的文件。
//2.正常情况下，推送端当有任务需要推送时，向接收端发送任务推送请求(包含任务号，推送任务目录名、文件数量、任务占用磁盘空间大小等信息)
// 并将文件会话 ID 与文件名的对应关系写在本地数据库中，以便重发时能识别要发送的文件名。
//3.发送端收到发送文件队列请求时，便可向接收端推送文件，任务完成后(所有文件推送完成)，向推送端发送任务完成通知。
// 等待重发失败文件，重复２~3步
//4.接收端收到推送文件信息时，将该文件信息写入配置文件
//备注：linux版的不做文件标记判断，只在Win版中做
//在Linux平台下编译
#ifdef OS_LINUX
#define LOCKFILE "/var/run/"                    //运行实例文件锁ID标志
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)            //文件锁类型

/*******************************************************************************
 * Function     : lockfile
 * Description  : 设置文件锁属性值
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int lockfile(int fd)
{
    struct flock fl = { 0 };
    fl.l_type = F_WRLCK;  // write lock
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;  //lock the whole file
    return (fcntl(fd, F_SETLK, &fl));
}

/*******************************************************************************
 * Function     : already_running
 * Description  :  判断程序实例是否已在运行
 * Input        :
 * Return       :
				0, 该文件没有被加锁
				1, 该文件已被加锁
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int already_running(const char *filename)
{
    if (NULL == filename)
        return -1;
    int fd = 0;
    fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0)
    {
        printf("can't open %s: %m\n", filename);
        exit(1);
    }
    //先获取文件锁
    if (lockfile(fd) == -1)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
        	printf("file: %s already locked\n", filename);
            close(fd);
            return 1;
        }
        printf("can't lock %s: %m\n", filename);
        close(fd);
        exit(1);
    }
    //写入运行实例的pid
    ftruncate(fd, 0);
    char buf[32] = { 0 };
    sprintf(buf, "%d", getpid());
    write(fd, buf, strlen(buf) + 1);
    //close(fd);　　　//在这里不用关闭文件，如果关闭文件后则文件锁功能就不存在了
    return 0;
}
#endif

/*******************************************************************************
 * Function     : ctrlhandler
 * Description  : 处理系统中断(ctrl+c)
 * Input        : fdwctrltype 中断信号量
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
#ifdef OS_WIN
bool ctrlhandler( unsigned long fdwctrltype )
{
    switch( fdwctrltype )
    {
        // handle the ctrl-c signal.
        case CTRL_C_EVENT:
        LOG_INFO( "ctrl-c event\n" );
        return( true );
        case CTRL_CLOSE_EVENT:
        LOG_INFO( "ctrl-close event\n\n" );
        if(g_AnalyseLayer)
        {
            g_AnalyseLayer->Release();
            SLEEP_MS(2000);
            delete g_AnalyseLayer;
            g_AnalyseLayer = NULL;
        }
        exit(1);
    }
}
#endif

/*******************************************************************************
 * Function     : HandleSig
 * Description  : 处理系统中断(ctrl+c)
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void HandleSig(int signo)
{ 
#ifdef OS_LINUX
    if (SIGINT == signo || SIGTERM == signo)
    {
        LOG_INFO("Rev Term sig!!");
        bMainStop = true;
        SendThread_Stop = true;
    	g_Socket->Close();
    }
#endif
}


/*******************************************************************************
 * Function     : Check_Running
 * Description  : 系统只允许运行一个进程
 * Input        :
 * Return       : 0,未有进程实例运行 -1,该进程已经在运行
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int Check_Running(const char *fname)
{
#ifdef OS_LINUX
	//整个系统只允许一个实例在运行
	string appdir(LOCKFILE);
	appdir += "/";
	appdir += basename(fname);
	appdir += ".pid";
	if (already_running(appdir.c_str()))
	{
		return -1;
	}

#endif
	//只允许运行一个实例
#ifdef OS_WIN
	HANDLE appInstance;
	//只允许运行一个实例
	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle = TRUE;
	sa.nLength= sizeof(sa);
	sa.lpSecurityDescriptor = NULL;  
	string appname((string(fname).substr(string(fname).find_last_of('\\')+1))); 
	appInstance = CreateMutex(&sa, FALSE, appname.c_str());
	//UNIQUE_NAME为定义的宏，此处也可以使用字符串
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(appInstance);
		appInstance = NULL;
		return -1;
	}
#endif
	return 0;
}

/*******************************************************************************
 * Function     : InitSysCtrl
 * Description  : 初始化系统控制功能
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void InitSysCtrl(void)
{
    std::locale::global(std::locale(""));
#ifdef OS_LINUX
    signal(SIGPIPE, SIG_IGN); //忽略PIPE中断,避免客户端退出而导致服务器也闪退现象
#endif
}
/*******************************************************************************
 * Function     : ScanAndMoveFilePro
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void ScanAndMoveFilePro(void)
{
    for(auto iTask : g_FileTask)
    {
		//程序启动时，需要先处理完该任务下的缓存目录文件，才能扫描源目录
    	if(iTask.second->IsEnable)
    	{
    		iTask.second->ScanAndMoveFile();
    	}
		//程序刚启动，先处理缓存目录,处理完毕后，给IsEnable标志位，不在进入该位置
    	else
    	{
    		iTask.second->IsEnable = iTask.second->HandTmpFile();
    	}
        SLEEP_S(1);
    }
}
/*******************************************************************************
 * Function     : SendFileProc
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void SendFileProc(void)
{
	LOG_DEBUG("SendPthread...");
	SendThread_Stop = false;
	while(!SendThread_Stop)
	{
		SLEEP_S(3);
	    for(auto mp : g_FileTask)
	    {
	    	mp.second->SendFile();
		}
	}
}

/*******************************************************************************
 * Function     : UpdateTaskInfo
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : -1 error
 * 				: >0 success
 *******************************************************************************/
int UpdateTaskFromSocket(void)
{
	if (g_AnalyseLayer->UpdateTask != 2)
		return 0;
    std::unique_lock <std::mutex> lck(main_Mutex);
	SendThread_Stop = true;
	if(SendThread.joinable())
	{
		LOG_INFO("Receive new task, wait to update to local...");
		SendThread.join();
	}
	g_FileTask.clear();
	for(auto task:g_ClientTaskList)
	{
		if (task.rule.nLimitSpeed < FILES_BUFSIZE_MIN || FILES_BUFSIZE_MAX < task.rule.nLimitSpeed)
		{
			LOG_WATCH(task.rule.nLimitSpeed);
			task.rule.nLimitSpeed = Config::Sender.nBufMaxCnt;
		}
		if(task.rule.nTransmod == OUT0INNER)
			g_FileTask[task.rule.nTaskID] = new SFileTask(task.rule);
		else
			g_FileTask[task.rule.nTaskID] = new FileTask(task.rule);
	}
	SendThread = std::thread(SendFileProc);
	g_AnalyseLayer->UpdateTask = 3;
	return g_AnalyseLayer->UpdateTask;
}
/*******************************************************************************
 main 函数执行流程：
 1.进程实例的唯一性检查
 2.初如化系统控制注册
 3.CA认证文件信息获取
 4.创建socket连接对象及连接资源
 5.等待与服务器登录验证状态
 6.根据不同的工作模式执行不同的工作（外端循环定期发送任务文件，内端接收文件）。
 *******************************************************************************/
int main(int argc, char* argv[])
{
	//一台电脑只允许运行一个同名实例
    if (Check_Running(argv[0]) < 0)         
        exit(-1);
    LOG_LEVEL_SET(argc, argv);

    InitSysCtrl();
	//Config
    if(Config::Set(argv[0]) < 0)
	{
		LOG_ERROR("Failure to get the configure from local !!!");
		SLEEP_S(5);
		return -1;
	}
	//Socket
	g_Socket = new Socket(Config::Com.strIp, Config::Com.nPort);
    if (NULL == g_Socket) return -1;
	if(g_Socket->ssl->Init(
			Config::Com.strKeyFilename,
			Config::Com.strCacrtFilename,
			Config::Com.strCrtFilename ) < 0)
	{
		LOG_ERROR("Failure to init Ssl，check the path please!!!");
		SLEEP_S(5);
		return -1;
	}
	if(!g_Socket->Start()){
		LOG_ERROR("Failure to start the socket，check the ip and port please!!!");
		SLEEP_S(5);
		return -1;
	}
	//Analyse
    g_AnalyseLayer = new Analyse(); //创建socket对象
    if (NULL == g_AnalyseLayer) return -1;
    g_AnalyseLayer->Start(); 
	//main
    while (!bMainStop)
    {
        SLEEP_S(1);
        if(!g_AnalyseLayer) break;
		if (!g_AnalyseLayer->m_bLogin){
			LOG_INFO("wait to login...");
	        SLEEP_S(2);
			continue;
		}
    	if(g_AnalyseLayer->UpdateTask < 3)
    	{
    		UpdateTaskFromSocket();
	        SLEEP_S(1);
    		continue;
    	}
    	ScanAndMoveFilePro();
    }
    return 0;
}

