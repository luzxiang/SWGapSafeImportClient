/******************************************************************************
 BlueDon  gap unilateral separation components head file.

 Copyright (C), 2017-2018, BlueDon Tech. Co., Ltd.
 Modification:  2017-10-23 Created
 author : wangxiaozhong
 ******************************************************************************/
#ifndef __BD_SWGAP_COMM_H__
#define __BD_SWGAP_COMM_H__

#define _DEBUG_VER                  1       //用于调试版本

#define PATH_LEN                    260     //目录长度
#define FILE_NAME_LEN               260     //文件名长度
#define USER_GROUP_NUM              4       //用户组数量
#define USERNAME_LEN                32      //用户名长度
#define PASSWORD_LEN                32      //密码长度
#define IP_LEN                      48      //IP地址长度
#define MAC_ADDR_LEN                64      //MAC地址长
#define IPV6_LEN                    64      //IPV6地址长度
#define DEV_INTERFACE_NUM           16      //设备接口数量
#define MAX_UDP_PACK_LEN            6500    //UDP包长度
#define MAX_SOCKET_BUF              8196    //SOCKET 缓冲区大小
#define MAX_FILE_BUF                1024*1024//文件缓冲区大小（定量存盘）
#define MANAGE_PORT                 3399    //主管理模块监听端口（各子模块与主管理模块的通信端口, 不提供外部网络连接）
#define SAFEIMPORT_PORT             4433    //安全导入模块监听端口
#define MAX_SAFE_CLIENT             1000    //最大安全导入客户端连接数
#define MAX_SAFE_SEND_THREAD        6       //最大安全导入模块推送文件线程数

#define TEST_TASKID                 98800   //内部通道通信状态测试任务号(该任务ID为系统专用，用户不能设置该ID号)
#define DBSTRUCT_TASKID             98880   //传输数据库表结构任务ID号 (该任务ID为系统专用，用户不能设置该ID号)
#define TASKINFO_TASKID             98881   //传输任务信息任务ID号 (该任务ID为系统专用，用户不能设置该ID号)
#define UDP_AGENT_TASKID            98882   //传输代理UDP数据任务ID号 (该任务ID为系统专用，用户不能设置该ID号)
#define TCP_AGENT_TASKID            98883   //传输代理TCP数据任务ID号 (该任务ID为系统专用，用户不能设置该ID号)

#define SAFEFILE_TASKID             8000    //安全文件传输任务
//文件存放路径外端：/home/temp/8000/用户目录/临时文件目录  其中用户名为当前登录用户帐号名，
//当文件传输到内端后需将临时文件目录传到相应的用户目录下，并删除临时文件及目录
//文件存放路径内端：/home/temp/8000/用户目录 其中用户名为外端传文件时的文件中的用户目录名，
//直接将临时文件目录传到相应的用户目录下，不存在临时目录

#define GAP_NOERROR                 0x00    //成功(无错误)
#define GAP_ERR_NOFILE              0x01    //没有文件
#define GAP_ERR_NODIR               0x02    //没有目录
#define GAP_ERR_OPENFILE            0x03    //打开文件失败
#define GAP_ERR_OPENDIR             0x04    //打开目录失败
#define GAP_ERR_MD5                 0x05    //MD5验证失败
#define GAP_ERR_VIRUS_INIT          0x06    //病毒库初始化失败
#define GAP_ERR_FTPLOGIN            0x07    //FTP服务登陆失败
#define GAP_ERR_SMBMOUNT            0x08    //smba挂载失败
#define GAP_ERR_NFSMOUNT            0x09    //NFS挂载失败
#define GAP_ERR_FTP_ACK             0x0A    //FTP服务无应答
#define GAP_ERR_FTP_GET             0x0B    //FTP下载文件失败
#define GAP_ERR_FTP_PUT             0x0C    //FTP上传文件失败
#define GAP_ERR_SMB_COPY            0x0D    //smba复制文件失败
#define GAP_ERR_NFS_COPY            0x0E    //nfs复制文件失败
#define GAP_ERR_EMAILLOGIN          0x0F    //登陆失败
#define GAP_ERR_GETEML_FAIL         0x10    //下载邮件失败
#define GAP_ERR_GETEMLID_FAIL       0x11    //获取邮件标识失败

#define SERV_STAT_YES 1 //可提供服务
#define SERV_STAT_NO 0  //不可提供服务

#define SWGAP_INNER 1 //网闸内端工作模式
#define SWGAP_OUTER 2 //网闸外端工作模式
//任务ID取值范围规则(自动生成，全系统惟一)
//#define SWGAP_RCV 	1  	//1：Receive
//#define SWGAP_SND 	2   //2：Send
//#define SWGAP_RS	3   //3：send and receive

/*
//用户组角色规定
//角色分为(三权分立 系统管理员 安全管理员  审计管理员)
//usermanager用户管理管理员:只能访问用户配置页面，有且仅有一个
//adminsys系统管理员:可以访问除用户配置之外的所有页面，具有最高权限
//manager管理员:能查看（日志审计，用户配置和登录设置）以外的所有页面，但没有修改权限
//logauditor日志审计员：只能查看和操作日志审计页面
*/
/*
 系统管理员：
 权限：具有最高权限（添加、删除文件（数据库、邮件）同步、配置安全文件传输服务全局参数；修改系统 IP 地址参数等功能）；
 其中只有 sysadmin 管理员具有创建、删除其他系统管理员账户权限，创建的其他系统管理员登陆后无此项功能。

 安全管理员：
 权限：包括：管理主机绑定 IP 与 Mac 地址；管理端口的绑定；登录的认证策略；添加安全文件传输账户、ＦＴＰ源帐户、数据源帐户配置，以及系统日志的查询、导出，　能查看（日志审计，用户配置和登录设置）以外的所有页面，但不能修改（前面列出的除外）；

 审计管理员：
 权限：对各类管理员的登陆及操作日志进行审计、导出等即只能查看日志；
 */

//蓝盾硬件网卡接口对应关系：
//单向传输口(光口):enp10s0
//管理口:enp2s0
//HA口:enp11s0
//业务口(电口):enp3s0、enp4s0、enp5s0、enp6s0
//业务口(光口):enp1s0f0、enp1s0f1、enp1s0f2、enp1s0f3
//结构1字节对齐
#pragma pack(1)

typedef enum
{
	SWGAP_SND=1,  //：Send
	SWGAP_RS,     //：send and receive
	SWGAP_RCV, 	//：Receive
} TRANSMOD;

//程序传输方向
typedef enum
{
    OUT2INNER = 0,    //0：外-->内
    INNER2OUT,      //1：内-->外
    OUT0INNER,      //2：外<-->内
} TRANSDIRT;

//报文类型枚举体
typedef enum
{
    FILETYPE_FILE,                       //普通文件
    FILETYPE_IMPORT_Client,              //导入客户端
    FILETYPE_DBFILE,                     //数据库文件
    FILETYPE_EMAILFILE,                  //邮件文件
    FILETYPE_HTTPFILE,                   //Http网页文件
    FILETYPE_MSG = 10,                     //定制消息
} FILETYPE;

//报文命令类型枚举体
typedef enum
{
    COMMAND_FILEINFO = 1,                 //文件信息包
    COMMAND_DATA = 2,                     //数据包
    COMMAND_FILE_STAT = 3,                //文件已发送状态
                                          //MSG 消息包根据需求再定(参见MSG_TYPE)
    COMMAND_MSG_TASK_STAT = 10,           //任务已发送状态

    COMMAND_MSG_TEST = 12,                //单向光模块通信链路状态测试包
} COMMANDCODE;

//消息类型枚举体

typedef enum
{
    MSG_TYPE_TASK_START = 1,            //任务传输开始(后接nTaskID所属任务ID号(内外端需对应))
    MSG_TYPE_TASK_FINISH,               //任务传输结束(后接nTaskID所属任务ID号(内外端需对应))
} MSG_TYPE;

//数据库类型枚举体
typedef enum
{
    DBTYPE_ORACLE = 0,                  //ORACLE数据库
    DBTYPE_SQLSERVER,                   //SQLSERVER数据库
    DBTYPE_MYSQL,                       //MYSQL数据库
    DBTYPE_POSTGRESQL,                  //PostgreSQL数据库
    DBTYPE_DB2,                         //DB2数据库
    DBTYPE_SYBASE,                      //SYBASE数据库
    DBTYPE_OSCAR,                       //OSCAR数据库
    DBTYPE_DM,                          //DM数据库
    DBTYPE_KINGBASEES,                  //KINGBASE
} DATABASE_TYPE;

//文件同步服务类型枚举体
typedef enum
{
    SYNCHRODATA_LOCAL_FTP = 0,             //本地FTP服务
    SYNCHRODATA_LOCAL_SMB,                 //本地SMB服务
    SYNCHRODATA_LOCAL_NFS,                 //本地NFS服务
    SYNCHRODATA_REMOTE_FTP,                //远端FTP服务
    SYNCHRODATA_REMOTE_SMB,                //远端SMB服务
    SYNCHRODATA_REMOTE_NFS,                //本地NFS服务
    SYNCHRODATA_CLIENT,                    //专用客户端
} FILE_SYNCHRODATA_TYPE;

//模块类型枚举体
typedef enum
{
    MODTYPE_SERVER = 0,                     //主程序管理模块名
    MODTYPE_TRANS,                          //数据发送/接收模块名
    MODTYPE_FILE_SYNC,                      //文件同步模块名
    MODTYPE_DB_SYNC,                        //数据库同步模块名：
    MODTYPE_EMAIL_SYNC,                     //邮件同步模块名：
    MODTYPE_WEBSITE_SYNC,                   //网站镜像模块名
    MODTYPE_LOG,                            //日志审计模块名：
    MODTYPE_MSG,                            //消息定制模块名:
    MODTYPE_FILE_CHECK,                     //文件核查模块名
    MODTYPE_MAINTENANCE,                    //维护管理模块
    MODTYPE_SAFE_IMPORT = 10,                    //安全导入模块
    MODTYPE_IMPORT_CLIENT,                  //专用安全导入客户端
    MODTYPE_IECLIENT,                       //IE客户端
    MODTYPE_FILE_BACKUP,                    //文件备份模块
    MODTYPE_DISK_FREE,                      //磁盘管理模块
    MODEYPE_HA_COMM,                        //双机热备文件传输(通信)模块
    MODEYPE_ALARM_NOTIFY,                   //告警通知模块
    MODEYPE_DDOS,                           //DDOS测试模块
    MODTYPE_AGENT,                          //定向传输模块(TCP/UDP定向代理)
    MODTYPE_FILE_SHARE,                     //文件共享中心模块
    MODTYPE_FILE_SHARE_CLIENT,              //文件共享中心客户端模块
    MODTYPE_WEB_CONFIG,                     //安全浏览配置转换模块
    MODTYPE_DB_AGENT,                       //数据库访问
    MODTYPE_FTP_AGENT,                      //FTP访问
    MODTYPE_MAIL_AGENT,                     //邮件访问
    MODTYPE_WEB_LOG,                        //安全浏览日志模块
} MODULE_TYPE;

//涉密等级
typedef enum
{
    SECRET_NORMAL = 0,                //公开
    SECRET_INNER,                     //内部
    SECRET_CLASSIFIED,                //秘密
    SECRET_CONFIDENTIAL,              //机密
    SECRET_TOPSECRET,                 //绝密
} SECRET_LEVEL;

//数据包头定义：
typedef struct tagPackHead
{
    unsigned short nVersion;     	 //版本号
    unsigned short nCodeType;   	 //报文类型
    unsigned short nCommandCode;         //命令类型
    unsigned short nHeadLen;      	 //包头长度
} PACK_HEAD;

//文件包结构体定义
typedef struct tagFileInfoPackHead
{
    unsigned int nSessionId;                  //会话ID(或任务号/文件编号，对于传输同一个文件该 ID号唯一)
    unsigned int nTaskID;                       //所属任务ID号(内外端一一对应)
    char strFileName[FILE_NAME_LEN + PATH_LEN];   //文件名(包含目录路径)
	char strFilePath[PATH_LEN];   //文件全路径
    unsigned long int nFileSize;                        //文件总长度(64位长度，大文件)
    unsigned short nWepType;          //文件加密类型(0, 不加密，1.IDEA 算法 2.RSA算法 3.AES算法)
    char strReserved[32];                       //保留字段(可作为加密用MD5)
    char bDir;                                  //文件类型(判断是其他文件还是文件夹)add by caotg
    unsigned int atime;				//访问时间距离当前时间的差值
	unsigned int mtime;				//修改时间距离当前时间的差值
} FILEINFOPACK_HEAD;

//数据包结构体定义(UDP传输方式)
typedef struct tagDataPackHead
{
    unsigned int nSessionId; 	     //会话ID(或任务号/文件编号，对于传输同一个文件该 ID号唯一)
    unsigned int nFileType;          //文件类型(参见枚举类型FILETYPE)
    unsigned int nTotalPackNum;      //包总数
    unsigned int nPackSeqNo;         //包序号
    long long nTotalLength;          //总长度(文件总长度)
    unsigned int nDataLength;        //数据长度(如果有RS校验，则不包含RS校验字节)
    unsigned short nRSCheck;         //是否需要RS前向纠错校验 0, 不需要
    char strReserved[32];            //保留字段
} DATAPACK_HEAD;

//数据包结构体定义(TCP传输方式)
typedef struct tagTCP_DataPackHead
{
    unsigned int nSessionId; 	     //会话ID(或任务号/文件编号，对于传输同一个文件该 ID号唯一)
    long long nTotalLength;          //总长度(文件总长度)
    unsigned int nLength;            //数据长度(如果有RS校验，则不包含RS校验字节)
} TCP_DATAPACK_HEAD;

#define MESSAGE_LEN    5505  // (MAX_UDP_PACK_LEN/255)*223-(sizeof(PACK_HEAD)+sizeof(DATAPACK_HEAD))  //每次读取文件内容数据长度(RS校验有效数据为233个, 另外32个校验数据)

//任务状态结构体定义
typedef struct tagTaskStatInfo
{
    int nTaskID;                        //任务编号
    int nState;                        //任务状态 为正数(0~100时表示任务进度值,为100时，表示任务已传送完成)
//为-1时表示任务中断(中断原因需详细定义)
    char strSubPath[32];
} TASK_STATE_INFO;

//系统状态结构体
typedef struct tagSystemStat
{
    float fCpuUsageRate;              //CPU使用率
    float fMemUsageRate;              //内存使用率
    float fDiskUsageRate;             //磁盘使用率
} SYSTEM_STAT;

//系统信息列表
typedef struct tagSystemInfo
{
    char strSystenName[64];             //系统名称
    char strSystenVer[16];              //系统版本号
    char strDBSynVer[16];               //数据库同步程序版本号
    char strFileSynVer[16];             //文件同步程序版本号
    char strEmailSynVer[16];            //邮件同步程序版本号
    char strWebSite[16];                //网站同步程序版本号
    int nRunTimes;                     //系统运行时长
    char strCurTime[20];                //当前时间
    char strLastUpdateTime[20];         //最后更改配置时间
} SYSTEM_INFO_;

//硬件接口状态信息
//可以有多条
typedef struct tagDevInterfaceStat
{
    char strDevName[32];                //设备名称(如网卡eth0~eth8 /enp1s0等)
    char strDevDescription[32];         //设备描述(如网卡的IP地址信息等)
    short nStat;                        //设备运行状态
} DEV_INTERFACE_STAT;

//设备接口状态列表
//DEV_INTERFACE_STAT　devInterfaceList[DEV_INTERFACE_NUM];

//系统时间结构体
typedef struct tagSystemime
{
    int iYear;			//年
    int iMonth;			//月
    int iDay;			//日
    int iHour;			//时
    int iMinute;		//分
    int iSecond;		//秒
} SYSTEM_TIME;

//用户分组（角色）结构体
typedef struct tagUserGroupRoles
{
    int nGroupID;                           //	用户组ID号,以下为各值说明：
                                            //  1.adminsys系统管理员:可以访问除用户配置之外的所有页面，具有最高权限；
                                            //  2.manager管理员:能查看（日志审计，用户配置和登录设置）以外的所有页面，但没有修改权限;
                                            //  3.logauditor日志审计员：只能查看和操作日志审计页面;
                                            //  4.usermanager用户管理管理员:只能访问用户配置页面，有且仅有一个；
                                            //  5.tester测试员
    int nLevel;                          //用户组级别(该用户组级别,值越小，所拥有的权限发挥高，即1为最高级别权限)
    char strGroupName[32];                  //用户组名称(参见GroupID之说明)
    int nFirstPage;                         //首页(为0表示不显示，1显示)
    int nSysConfig;                //系统配置(为0不显示，其每一位BIT 值表示显示某项内容： 0x1 用户管理 0x2)
    int nAvailability;                      //高可用性
    int nNetWork;                           //网络设置
    int nVirus;                             //病毒防护
    int nObjectDefine;                      //对象定义
    int nDBSync;                            //数据库同步
    int nFileSync;                          //文件同步
    int nEmaiSync;                          //邮件同步
    int nLogBrowse;                         //日志审计
} USER_GROUP_ROLES;

//用户信息结构体
typedef struct tagUserInfo
{
    int nID;                                //用户ID
    int nGroupNo;                           //所属用户组号
    bool bEnable;                           //是否启用该用户
    char strUserName[USERNAME_LEN];         //用户名(密码采用MD5加密)
    char strPassword[PASSWORD_LEN];         //登录密码(密码采用MD5加密)
    int strIPV4[IP_LEN];                    //IPV4地址
    char strMAC[MAC_ADDR_LEN];              //MAC地址
    char strIPV6[IPV6_LEN];                 //IPV6地址
    short nBindMac;                         //绑定mac地址(是否绑定mac地址，1是，0否)
} USER_INFO;

//Web控制管理结构体
typedef struct tagWebCtrlManage
{
    short nMaxOnlineTime;                   //在线（会话）超时时长限制(单位分,默认60分钟)
    short nMaxOnlineUser;                   //最大并发管理用户数量(默认10)
    short nLoginFailTimes;                  //登录错误次数限制(默认3次)
    short nLockTime;                        //封锁IP时间(	默认1分钟)
    int nLoginPort;                       //登录端口(https：默认443)
    bool bRemotManage;                      //启用远程管理(默认 false)
    char strPermissionIPV4[256];            //允许登录的白名单IPV4	(IP地址 IP地址之间用";"隔开，
                                            //须支持网段模式如：192.168.0.10~28 表示允许该网段 10至28的IP都允许访问)
    char strPermissionIPV6[512]; //允许登录的白名单IPV6	Varchar(512)	IP地址 IP地址之间用";"隔开，
                                 //须支持网段模式如：192.168.0.10~28 表示允许该网段 10至28的IP都允许访问
    bool bDisbleIP[256];                    //禁止登录的黑名单IP地址 IP地址之间用";"隔开，
                                            //须支持网段模式如：192.168.0.10~28 表示允许该网段 10至28的IP都允许访问(本项内容暂时保留)
    char strRootCA[256];                    //根证书文件名:根证书文件需下载到配置程序的相应目录中
} WEB_CTRL_MANGE;

//NTP时钟服务器结构体
typedef struct tagNtpServer
{
    char strNtpAddr_1[IP_LEN];               //NTP服务器地址1
    char strNtpAddr_2[IP_LEN];               //NTP服务器地址2
    char strNtpAddr_3[IP_LEN];               //NTP服务器地址3
    char strDomain[128];                     //域名
    int nServerPort;                         //NTP服务端口(默认 UDP 123|C. TCP 132)
    short nServerFlag;                       //服务标志(1,TCP ,2 UDP)
    int nInterval;                           //同步间隔(时间单位：秒)
} NTP_SERVER;

//时区结构体
typedef struct tagTimeZone
{
    int nTimeZone;                           //时区
    char strTimeZoneName[32];                //时区名称
} TIME_ZONE;

//产品许可证结构体
typedef struct tagProductionLicence
{
    char strlicence[256];                    //产品许可证
    int nRunModul;                           //启用模块
} PRODUCTION_LICENCE;

//HA端口配置
typedef struct tagHaInterface
{
    short nHaType;                           //HA模式(默认双机热备)
    short nHaStat;                           //HA状态(主状态 备状态)
    short nHaPriority;                       //HA优先级
    char strlcoalIP[IP_LEN];                 //本机心跳IP
    char strDestIP[IP_LEN];                  //对端心跳IP
    short nSyncType;                         //同步模式(1， 同步流  2，IPSec)
} HA_INTERFACE;

//双机热备配置结构体
typedef struct tagDoubleHot
{
    short nStatus;                           //状态
    char strVrid[128];                       //虚拟路由ID
    short nPriority;                         //优先级
    char strInterfaceName[32];               //网卡名称
    char strIpAddr[IP_LEN];                  //网卡对应的IP
    char strFloatIP[IP_LEN];                 //对外IP(浮动IP,或虚拟IP)
    char strDescription[64];                 //描述
} DOUBLE_HOT;

//双机热备资料备份信息结构体
typedef struct tagDoubleHotBackup
{
    bool bEnable;                            //是否启用双机热备服务
    char strStandbyIP[IP_LEN];               //备用机IP地址
    char strStandbyUser[USERNAME_LEN];       //备用机登录用户(MD5加密)
    char strStandbyPassword[PASSWORD_LEN];   //备用机登录密码(MD5加密)
    bool bUserManage;                        //用户管理(同步用户管理资料)
    bool bLoginManage;                       //登录管理(同步登录管理资料)
    bool bStaticRoute;                       //静态路由(同步静态路由资料)
    bool bVirtualIP;                         //虚拟I(同步虚拟IP资料)
    bool bCommCtrl;                          //访问控制(同步访问控制资料)
    bool bForceCtrl;                         //强制控制访问(同步强制控制访问资料)
    bool bFtpServer;                         //FTP服务(同步FTP服务资料)
    bool bSmbServer;                         //SMB服务(同步SMB服务资料)
    bool bDBSource;                          //数据资源(同步数据资源资料)
    bool bServerReg;                         //注册服务(同步注册服务资料)
} DOUBLE_HOT_BACKUP;

//网络接口配置结构体
typedef struct tagNetportInfo
{
    int nId;                                 //ID号
    char strPortName[32];                    //网络端口名称(eth1~eth8)
    char strNetMask[IP_LEN];                 //子网掩码
    char strWorkMode[16];                    //工作模式(路由模式;冗余模式)
    bool bByManagement;                      //是否用于管理(0否，1是)
    bool bAllowPing;                         //是否允许ping(0否，1是)
    bool bAllowTraceRoute;                   //是否允许traceroute(0否，1是)
    short nIPV4ConnectionType;               //ipv4连接类型(0静态 1 动态)
    char strIPV4Address[32];                 //ipv4地址
    char strIPV4NextJump[32];                //ipv4下一跳
    short nIPV6ConnectionType;               //Ipv6连接类型(0静态 1 动态)
    char strIPV6Address[64];                 //Ipv6地址
    char strIPV6NextJump[64];                //IPV6下一跳
    short nStatus;                           //状态(是否已启动 0,未启用，1已启用)
} NETPORT_INFO;

//网络端口聚合配置结构体
typedef struct tagPortAggregation
{
    int nId;                                 //ID号
    char strBridgeName[32];                  //端口聚合名称(eth1~eth8)
    char strBindDevices[256];                //绑定设备列表(设备名称用';'隔开)
    bool bByManagement;                      //是否用于管理(0否，1是')
    bool bAllowPing;                         //是否允许ping(0否，1是)
    bool bAllowTraceRoute;                   //是否允许traceroute(0否，1是)
    short IPV4ConnectionType;                //ipv4连接类型(0静态 1 动态)
    char strIPV4Address[32];                 //ipv4地址
    char IPV4Mask[32];                       //ipv4掩码
    short nIPV6ConnectionType;               //Ipv6连接类型(0静态 1 动态)
    char strIPV6Address[64];                 //Ipv6地址
    char strIPV6Mask[64];                    //IPV6掩码
    short nStatus;                           //状态(是否已启动 0,未启用，1已启用)
    char strWorkMode[16];                    //工作模式(路由模式 ；冗余模式)
    char strGetIP[IP_LEN];                   //IP地址获取
} PORT_AGGREGATION;

//默认网关信息结构体
typedef struct tagGatewayInfo
{
    char strName[32];                        //名称
    char strGatewayIP[IP_LEN];               //网关IP地址
    char strDescription[32];                 //描述
} GATEWAY_INFO;

//静态路由配置结构体
typedef struct tagStaticRoute
{
    int nId;                                 //ID号(自增)
    char strInterfacePort[32];               //网络接口名称
    char strTargetAddress[32];               //目标IP地址
    char strNetmask[IP_LEN];                 //子网掩码
    char strNextJumpIp[IP_LEN];              //下一跳(网关)
    bool bStatus;                            //状态(True:已启动， false 未启动)
    char strDescription[32];                 //描述
} STATIC_ROUTE;

//内部通道配置结构体(annelinfo) （主要用于单向传输外端与内端光口配置）
//只有一条记录
typedef struct tagLocalChannelinfo
{
    char strInterFaceName[32];			//外端网络接口名称
    char strInterfacePort[32];			//外端网络接口名称(eth0~eth7)
    char strIP[IP_LEN];                         //外端IP地址
    char strNetmask[IP_LEN];			//子网掩码
    char strInnerInterfacePort[32];		//内端端网络接口名称(eth0~eth7)
    char strInnerIP[IP_LEN];			//内端IP地址
    char strInnerMACAddr[32];			//内端MAC地址
} LOCAL_CHANNELINFO;

//IP/MAC地址绑定表(bind_ip_mac)
typedef struct tagBindIpMac
{
    int nId;                                    //ID号
    char strIP[IP_LEN];                         //IP地址
    char strMACAddr[32];                        //MAC地址
    char strDescription[64];                    //描述
} BIND_IP_MAC;

//病毒库升级信息结构体(virus_update)
typedef struct tagVirusUpdate
{
    bool bEnable;                          //是否启动查杀病毒引擎(True:启用查杀病毒引擎;false:不启用)
    bool bVirusDate;                            //病毒库日期信息
    char strVersion[32];                        //病毒库版本信息
    char strVirusFileName[260];                 //病毒库文件(目录该文件应放置在相当的病毒库目录中)
    bool bAutoUpdate;                           //是否自动升级(True 自动升级， false不自动升级)
    char strUpdateServerIP[IP_LEN];             //升级服务器地址
    int nUpdateServerPort;                      //升级服务器端口
    short nUpdateTimer;                         //定时升级类型(1按小时，2按天，3按周)
    char strTimerValue[128];                    //定时升级时间值
} VIRUS_UPDATE;

//病毒查杀策略结构体(virus_rule)
typedef struct tagVirusRule
{
    int nId;                                    //ID号
    char strRolename[64];                       //策略名称
    short nkillType;                            //处理动作(1隔离 2删除)
} VIRUS_RULE;

//业务注册信息结构体(service_register)
typedef struct tagServiceRegister
{
    int nId;                                    //访问控制ID
    char strServerName[64];                     //服务名称(如**文件同步、**数据库同步、**邮件同步等)
    char strServerUnit[64];                     //服务单位(所属单位　如研发中心等)
    char strServerType[64];                     //服务类型
    short nDataType;                      //数据类型(0可配置数据库同步 1可配置文件同步 2 可配置数据库和文件)
} SERVICE_REGISTER;

//时间定义结构体(time_plan)
typedef struct tagTimePlan
{
    int nId;                                    //ID号
    char strName[128];                          //时间计划名称
    char strStartTime[20];                      //开始时间
    char strEndTime[20];                        //结束时间timestamp(6)
} TIME_PLAN;

//本地通信控制访问结构体(netinterface_info)
typedef struct tagLocalCommCtrl
{
    int nId;                                    //通信控制ID号
    bool bEnable;                              //启有该规则(是否启用此规则 true 启用 false不启用)
    int nRightType;                             //动作类型(1 ACCEPT 2 DROP 3 REJEC)
    int nProtocolType;                          //协议类型(1 TCP 2 UDP 3 ICMP)
    int nSrcType;                               //源类型(1, any  2,单个主机)
    char strSrcIP[IP_LEN];                      //源IP地址
    int nSrcPort;                               //源端口号
    int nDestType;                              //目标类型(1, any  2,单个主机)
    char strDestIP[IP_LEN];                     //目标IP地址
    int nDestPort;                              //目标端口号
} LOCAL_COMM_CTRL;

//本地UDP通信访问结构件
//可以允许创建多条规则
typedef struct tagLocalUdpCommCtrl
{
    int nId;
    bool bEnable;                               //是否启用此规则
    char strDestIP[IP_LEN];                     //目的IP地址
    int nDestPort;                              //目的转发端口号
} LOCAL_UDP_COMM_CTRL;

//本地FTP服务结构体
//可创建个条（多个用户，每个用户可自动创建一个FTP目录）
typedef struct tagLocalFtpServer
{
    int nId;                             //ID号
    char strUserName[USERNAME_LEN];      //访问用户
    char strPassword[PASSWORD_LEN];      //访问密码
    int nUserGroupNo;                    //强制访问用户组，当为0时为不强制
    char strPath[64];                    //FTP目录
    short nRunState;                     //运行状态  0,停止  1运行
    short nRights;                       //操作权限  1，读 2，写 3,删除
} LOCAL_TFPSERVER;

//用户组文件操作安全等级
typedef struct tagGroupFileSecurity
{
    short nGroupNo;                      //用户组号
    short nUpLevel;                      //上传安全等级
    short nDownLevel;                    //下载安全等级
} GROUP_FILE_SECURITY;

//FTP允计操作的命令结构体
typedef struct tagFtpRights
{
    bool bUser;     //用户名
    bool bPass;     //密码
    bool bAcct;     //接收
    bool bCwd;      //改变工作路径
    bool bCdup;     //返回上一级目录
    bool bSmnt;     //结构加载
    bool bQuit;     //用户退出
    bool bRein;     //重新初始化
    bool bPort;     //主动
    bool bPasv;     //被动
    bool bType;     //类型
    bool bStru;     //文件结构
    bool bMode;     //传输模式
    bool bRetr;     //下载文件
    bool bStor;     //保存文件
    bool bStou;     //唯一保存
    bool bAppe;     //追加或者新建文件
    bool bAllo;     //分配
    bool bRest;     //重新开始
    bool bRnfr;     //重命名
    bool bRnto;     //重命名为
    bool bAbor;     //放弃
    bool bDele;     //删除文件
    bool bRmd;      //删除目录
    bool bMkd;      //创建目录
    bool bPwd;      //打印目录
    bool bList;     //列表
    bool bNlst;     //名字列表
    bool bSite;     //站点参数
    bool bSyst;     //系统
    bool bStat;     //状态
    bool bHelp;     //帮助
    bool bNoop;     //等待
} FTP_RIGHTS;

//FTP强制控制访问结构体
typedef struct tagFtpForceCtrl
{
    bool bEnable;                        //是否启用
    char strPermissionIP[256]; //允许访问FTP服务的白名单（IP地址 IP地址之间用";"隔开，须支持网段模式如：192.168.0.10~28 表示允许该网段 10至28的IP都允许访问）
    char nFileSecurityLevel1[10];        //文件第1安全等级标志
    char nFileSecurityLevel2[10];        //文件第2安全等级标志
    char nFileSecurityLevel3[10];        //文件第3安全等级标志
    char nFileSecurityLevel4[10];        //文件第4安全等级标志
    GROUP_FILE_SECURITY GroupFileSecurity[USER_GROUP_NUM];   //用户组文件操作安全等级
    FTP_RIGHTS FtpRights;                //FTP允计操作的命令
} FTP_FORCE_CTRL;

//本地SMB 服务结构体
//可创建多个SMB服务（多个用户，每个用户可自动创建一个SMB目录）
typedef struct tagLocalSmbServer
{
    char strUserName[USERNAME_LEN];      //访问用户
    char strPassword[PASSWORD_LEN];      //访问密码
    char strPath[64];                    //SMB目录
    short nRunState;                     //运行状态  0,停止  1运行
    short nRights;                       //操作权限  1，读 2，写 3,删除
    short nProtocol;                     //协议     1 SMB协议 其它未定
} LOCAL_SMBSERVER;

//NFS 服务结构体
//可创建多个服务
typedef struct tagLocalNfsServer
{
    char strServerName[32];              //服务名
    char strPath[64];                    //NFS目录
    short nRunState;                     //运行状态  0,停止  1运行
    short nRights;                       //操作权限  1，读 2，写 3,删除
    char strIP[256];                     //IP过滤
} LOCAL_NFSSERVER;

//数据源信息结构体
//(可以有多条记录）
typedef struct tagDBSrcConfInfo
{
    int nID;                            //ID号
    char strSrcName[64];                //资源名称
    short nSrcType;                     //资源类型
    short dbType;                       //数据库类型 参见DATABASE_TYPE 枚举
    char strDbIp[IP_LEN];               //数据库所在的IP地址
    int nPort;                          //数据库服务端口(默认端口参见以下所列)
                                        //ORACLE 1521
                                        //SQLSERVER 1433
                                        //MYSQL 3306
                                        //DB2   50000
                                        //SYBase 5000
                                        //POSTGRESQL 5432
                                        //DM 5236
                                        //OSCAR  2003
                                        //KingbaseES  54321
    char strUserName1[USERNAME_LEN];    //用于同步帐号用户名
    char strPassword1[PASSWORD_LEN];    //用于同步帐号密码名
    char strUserName2[USERNAME_LEN];    //用于业务帐号用户名
    char strPassword2[PASSWORD_LEN];    //用于业务帐号密码名
    char strDabaseName[32];             //数据库名称
    char strModeName[32];               //模式名
} DBSRC_CONFINFO;

//数据库同步任务信息结构体(dbsync_server)
typedef struct tagDbsyncServer
{
    int nId;                            //ID号
    int nLinkRegServerID;               //关联注册服务（业务）ID号(关联业务注册信息表中的ID号)
    int nTaskID;    //服务（任务）ID号(系统依靠本ID号来对文件同步服务的唯一标志，因此内网与外网该ID号必须一致（该ID号全系统惟一）
    char strTaskName[64];               //任务名称
    short nLevel;                       //任务等级，值越小等级越高，可优先传输。
    short nLinkkillvirus;               //查杀病毒策略(关联病毒查杀策略表中的ID号)
    char strKeyWordFilter[256];         //关键字过滤(关键字间用';'隔开，支持模糊过滤功能)
    bool bEnable;                       //是否生效(True,启用，  false:不启用)
    int nLinkDoTime;                    //生效时段(关联时间定义表中的ID号)
    char strDescription[128];           //描述
} DBSYNC_SERVER;

//数据库同步策略结构体(dbsync_rule)
//可以有多条记录
typedef struct tagDbsyncRule
{
    int nId;                            //ID号
    char strRuleName[128];              //策略名称
    int nLinkTaskID;                    //关联服务ID号(关联数据库任务信息表中的TaskID)
    int nLinkDbsrc;                     //关联数据源(关联数据源表中的ID号)
    short nSynType; //同步方式(1 触发器方式是增量同步，为了达到同步效果需保证建立任务之前两边表数据已一致 , 2 普通全表，3.异构同步)
    bool bNeedSignRight;                //是否开启签权(True:开启 false:未开启)
    char strSignRightIP[IP_LEN];        //签权客户IP地址
    char strSignRightInfo[128];         //签权信息
    char strOuterSrcName[64];        //外端资源名称(参见结构DBSRC_CONFINFO，之strSrcName之定义)
    char strInnerSrcName[64];        //内端资源名称(参见结构DBSRC_CONFINFO，之strSrcName之定义)
    int nSynTimeType;                   //同步方式(1,时间间隔 2, 定时 3 其它)
    int nSynTimer;                  //定时同步周期(仅对nSynTimeType为3时有效 (1每天 2每周  3每月）)
    char strSynDate[64]; //同步日期(仅对nSynTimeType为3时有效 (1每天 2每周（周一至周日可选） 3每月 输入范围为1-31，多个以空格分隔，例：1 28代表每月1号和28号)
    char strSynTime[20]; //同步时间(当nSynTimeType 为1时间间隔时,  单位为秒 当nSynTimeType 为2 定时时 格式为 yyyy-MM-dd hh:mm:ss  当nSynTimeType定时3 其它时 格式为 hh:mm:ss)
} DBSYNC_RULE;

//数据库同步表信息列表
typedef struct tagDbSyncTableInfo
{
    //int nId;
    int nLinkRuleID;                    //关联数据库库同步策略ID号
    char strTaskTableName[64];          //任务表命名
    char strExternTableName[64];        //外端数据库表名
    char strInnerTableName[64];         //内端数据库表名
    int nSynSortNo;                    //同步顺序号
    bool bSameTable;                    //表结构是否一致
    bool bIfUpdate;                     //是否按条件更新
    char strIfInfo[128];                //条件表达式
    char strOuterNewFieldName[64];      //外端新增字段名
    char strInsideNewFieldName[64];     //内端新增字段名
    char strWhereSql[128]; //源端条件过滤where子句：（注意：where子句里面的字段名前请加上"t.",例如:"t.id < 10"）
    char strFieldSync[512];        //按字段同步(格式:外端字段名1,内端字段名1; ... 外端字段名n,内端字段名n;)
} DBSYNC_TABLEINFO;

//文件源配置结构体(filesrc_confinfo)
typedef struct tagFileSrcConfigure
{
    int nID;                            //ID号
    char strServiceName[64];            //服务名称
    short nSrcType;                     //资源类型参见FILE_SYNCHRODATA_TYPE,
    char strSrcPath[128];               //源目录(资源所在目录)
    char strSrcIP[IP_LEN]; //源IP地址(资源所在源IP地址，当srcType为SYNCHRODATA_CLIENT时，表示客户端用户登录所在的IP地址)
    int nSrcPort;                       //服务端口
    char strDescription[64];            //描述
    int nSecretLevel;                   //涉密级别
} FILESRC_CONFIGURE;

//文件核查模板结构体(filecheck_templet)
typedef struct tagFilecheckTemplet
{
    int nID;                            //ID号
    char strTempletName[64];            //模板名称
    bool bCheckFileExtension;           //文件扩展名核查(True:启用， false:不启用)
    char strAllowFileExtension[256];	//允许通过同步的扩展名(允许通过同步的扩展名，多个扩展名间用';'号隔开)
    bool bCheckKeyword;                 //关键字核查(True:启用， false:不启用)
    char strForbidKeyword[256];       //禁止的关键字(禁止通过的关键字，多个关键字间用';'号隔开。允许模糊查询“*”)
    bool bCheckFileSize;                //文件大小核查(True:启用， false:不启用)
    int nMinFileLen;                    //过滤最小字节数(单位：M, )
    int nMaxFileLen;                //过滤最大字节数(单位：B大于此数值可执行同步操作yte, 小于此数值可执行同步操作)
    bool bcheckmd5;                     //是否检验md5
} FILECHECK_TEMPLET;

//文件同步策略信息结构体
//(可以有多条)
typedef struct tagFilesyncRule
{
    int nId;                            //ID号
    int nTaskID;                        //任务(服务)ID号(系统依靠本ID号来对文件同步服务的唯一标志，因此内网与外网该ID号必须一致)
                                        //外端服务ID号惟一，只能允许一条记录，而内端ID可以多个，对应不同的同步对像，即一对多的关系

    int nSecretLevel;                       //涉密级别
    char strSrcPath[PATH_LEN];              //任务所处的目录
    int nSyncTime;                          //同步周期(单位秒:3~3600);

    int nLinkRegServerID;               //关联注册服务ID(与synchrodataregister表的ID号关联，表示该服务属于某注册服务下
    int nlinkfilesrcID;                 //关联文件资源ID号(关联filesrc_confinfo　表中ID号)
    char strTaskName[64];               //服务名称
    int nTransmod;                      //传输方向
    int srcType;                        //资源类型（参见 FILE_SYNCHRODATA_TYPE 枚举）
    short nLevel;                       //任务等级(值越小等级越高，可优先传输)
    short nEncodeing;                   //文件编码类型  UTF-8, GBK 其它待定
    int nLinkFileCheckId;               //关联文件核查模板(关联文件核查模块表中的ID号)
    int nLinkVirusRuleId;               //关联病毒查杀ID号(关联病毒查杀策略表中的ID号)
    short nAfterSended;                 //文件同步后的动作(1删除、2复制)
    bool bBackup;                       //启用备份
    int nBackupSrcTypeType; 		//备份资源类型 参见 FILE_SYNCHRODATA_TYPE 枚举 (只能是SYNCHRODATA_REMOTE_FTP（远端FTP服务）和 SYNCHRODATA_REMOTE_SMB远端SMB服务)
    int nBackup_linkfilesrcID;          //关联文件资源ID号(关联filesrc_confinfo　表中ＩＤ号)
    bool bDelBackup;                    //是否启用备份定期删除
    char strSrcUserName[USERNAME_LEN];	//登录用户名(MD5加密)
    char strSrcPassword[PASSWORD_LEN];	//登录密码(MD5加密)
    short nDelTimer;                    //备份定期删除周期  (1每天 2每周 3每月 )
    char strDelDate[64];                //删除日期  (1每天 2每周（周一至周日可选） 3每月 输入范围为1-31，多个以空格分隔，例：1 28代表每月1号和28号)
    char strDelTime[20];                //删除时间
    unsigned int nLimitSpeed;                    //网络限速（KB)
    bool bTimerSyn;                     //是否定时同步
    short nSynTime;                     //临时作为文件稳定时间 定时同步周期 (1每天 2每周  3每月）
    char strSynDate[64];                //同步日期(1每天 2每周（周一至周日可选） 3每月 输入范围为1-31，多个以空格分隔，例：1 28代表每月1号和28号)
    char strSynTime[20];                //同步时间
    char strFileType[100];                //file type
    int status;                         //状态 1启用  0不启用
} FILESYNC_RULE;

//外端接收邮件策略配置结构体
typedef struct tagOuterEmaiRecConf
{
    int nTaskID;                        //策略id
    char strAddress[64];                //策略对应收信箱
    char strPassword[32];           	//收信箱密码
    char strServerAddr[64];         	//收信箱服务器地址
    int nPort;                          //收信箱服务器端
    bool nStatus;                    	//当前状态
    int TimeID;

} OUTER_EMAILREV_CONF;

//内端发送邮件策略配置结构体
typedef struct tagInner_EmailSender
{
    int nTaskID;                        //策略id
    char strAddress[64];                //策略对应发件箱
    char strPassword[32];           	//发件箱密码（明文）
    char strServeraddr[64];             //发件箱服务器地址
    int nPort;                      	//发件箱服务器端
    int nStatus;                    	//当前状态
} INNER_EMAIL_SENDER;

//内端接收邮件策略表mail_rec_inner
typedef struct tagInner_EmailRecvConf
{
    int nTaskID;                    	//策略id
    char strAddress[64];            	//收件邮箱
    int nStatus;                    	//当前状态
} INNER_EMAIL_RECVCONF;

//通用日志常规设置结构体
typedef struct tagCommonLogConf
{
    bool bLogEnable;                    //启用日志
    bool bSyslogServer;                 //启动远程syslog服务器
    char strSyslogServerIP[IP_LEN];     //远程服务器IP地址
    int nSyslogServerPort;             //远程服务器端口
    short nLogFullOperateType;          //日志满操作方式 1,覆盖　2.暂停
    short nLogFullEmai;                 //日志满邮件提醒	1,邮件提醒，0,不提醒
    short nAlartEmail;                  //告警邮件提醒1,邮件提醒，0,不提醒
    char strEmailAddr[64];              //Email账号
    char strSMTPIP[IP_LEN];             //SNMP服务器IP地址
    int nSMTPPort;                      //SMTP服务器端口号
    char strSMTPPass[PASSWORD_LEN];     //密码
    int nLogLen;                        //日志存储文件大小(KB)
    int nLogAlarmLen;                   //报警阀值(大于该值即告警)
    short nCpuAlarm;                    //Cpu告警阀值(默认80,即使用到80%即告警)
    short nMemAlarm;                    //内存告警阀值(默认80,即使用到80%即告警)
    short nDiskAlarm;                   //磁盘告警阀值(默认80,即使用到80%即告警)
} COMMONLOG_CONF;

//系统登录日志结构体(syslog_login)
typedef struct tagSyslog_login
{
    int nId;                            //ID号
    char strloginIP[IP_LEN];            //登录IP
    char strloginUserName[USERNAME_LEN];            //登录用户名
    char strloginTime[20];              //登录时间
    short nStatus;                      //状态 1,成功　2失败
    char strloginOutTime[20];           //退出时间
    char strLoginfo[128];               //日志信息
} SYSLOG_LOGIN;

//系统备份/恢复日志结构体(syslog_backup)
typedef struct tagSyslogBackup
{
    int nId;                            //ID号
    char strIP[IP_LEN];                 //操作IP
    char strUserName[USERNAME_LEN];     //用户名
    char strLoginTime[20];              //记录时间
    short nOperateType;                 //操作类型1,备份　2恢复
    char strLoginfo[128];               //日志信息
} SYSLOG_BACKUP;

//系统重启/关机日志结构体(syslog_reboot)
typedef struct tagSyslogReboot
{
    int nId;                            //ID号
    char strIP[IP_LEN];                 //操作IP
    char strUserName[USERNAME_LEN];     //用户名
    char strLoginTime[20];              //记录时间
    short nOperateType;                 //操作类型1,重启　2关机
    char strLoginfo[128];               //日志信息
} SYSLOG_REBOOT;

//系统增加/删除用户日志结构体(syslog_user_manage)
typedef struct tagSyslogUserManage
{
    int nId;                            //ID号
    char strIP[IP_LEN];                 //操作IP
    char strUserName[USERNAME_LEN];     //用户名
    char strLoginTime[20];              //记录时间
    short nOperateType;                 //操作类型1,增加用户　2删除用户　3更改用户
    char strLinkUser[USERNAME_LEN];     //关联的用户名称
    char strLoginfo[128];               //日志信息
} SYSLOG_USER_MANAGE;

//系统资源告警日志表(syslog_resource)
typedef struct tagSyslogResource
{
    int nId;                            //ID号
    char strLogTime[20];                //记录时间
    short nAlarmType;                   //告警类型1,CPU　2内存　3磁盘
    char strLoginfo[128];               //日志信息
} SYSLOG_RESOURCE;

//数据库管理日志结构体
typedef struct tagManageLogInfo
{
    int nId;
    char strUserName[USERNAME_LEN];     //管理员名
    char strSrcIP[IP_LEN];            //源IP地址,(格式：192.168.0.11:2355   IP + port)
    char strConfType[32];               //操作配置类型（如登录管理界面、查看用户配置、创建服务、关闭系统等）
    char strLevel[16];                  //级别:告警 、　信息
    char strConfTime[20];               //配置时间
    char strLogInfo[128];               //具体的日志信息
} MANAGE_LOGINFO;

//数据库抽取日志结构体
typedef struct tagDBExtractLogInfo
{
    int nId;                            //记录ID号
    char strUserName[USERNAME_LEN];     //管理员名
    char strServerName[32];             //服务名称
    char strTableName[64];              //抽取的表名
    char strDataTo[32];                 //数据流向
    char strExtactTime[20];             //抽取时间
    char strLogInfo[128];               //抽取信息（包含Select语句）
} DBEXTRACT_LOGINFO;

//数据库基本信息日志结构体
typedef struct tagDbBaseLogInfo
{
    int nId;                            //记录ID号
    char strIP[IP_LEN];                 //设备IP
    char strSeverName[32];              //业务流程名（自定）
    char strEvent[32];                  //事件（打开会话、关闭会话、发送消息、接收消息等自定义）
    int nLevel;                         //等级
    char strTime[20];                   //发生时间
    char strLoginInfo[128];             //日志信息
} DBBASE_LOGINFO;

//文件同步信息日志结构体
typedef struct tagFileSyncLogInfo
{
    int nId;                               //记录ID号
    int nDirection;                             //内外端标志位 2外端  1 内端  0 不区分
    int nTaskID;                            //服务ID号(系统依靠本ID号来对文件同步服务的唯一标志，因此内网与外网该ID号必须一致)
    int nTid;
    char strPath[1024];                     //目录名
    char strFileName[1024];                 //文件名
    int nFilesize;                              //文件大小 以KB为单位
    char strLoginfo[1024];                      //日志内容
    char strLogtime[64];                        //日志时间
    int nStatus;                        //操作状态 (0 同步失败　1,同步成功　2正在同步中)
    char sql[1024*2];
} FILESYNC_LOGINFO;

//数据库同步信息日志结构体
typedef struct tagDBSyncLogInfo
{
    int nId;                            //记录ID号
    int nTaskID;                        //服务ID号(系统依靠本ID号来对文件同步服务的唯一标志，因此内网与外网该ID号必须一致)
    int nLinkDBSrcID;                   //关联数据库源
    int nDirection;							 //内外端标致 内端：1   外端：2  不区分内外端：0
    char strTableName[64];              //表名
    char strOperateType[32];            //操作类型(Insert、delete、update等)
    char strKeyword[64];                //主键如'name'=123456
    short nStatus;                      //操作状态 (0 同步失败　1,同步成功　2正在同步中)
    char strLastTime[20];               //最后操作时间
    char strLogInfo[128];               //日志信息
    int nSyncRecords;                   //同步记录数
    int nFailRecords;                   //失败记录数
    int nDataSize;                      //传输数据字节(以KB为单位)
} DBSYNC_LOGINFO;

//邮件传输日志信息表
typedef struct tagEmailSyncLogInfo
{
    int nID;                            	//ID号	Serial	主键,自增，not null
    int nDirection;								//内外端标志位
    int nTaskID;                        	//任务ID号
    char strSendaddress[64];					//发件箱
    char strDestaddress[64];              	//收件箱
    int nLimttype;									//限制类型
    int nIssuccess;                       	//是否发送成功
    char strLoginfo[512];               	//日志信息
    char strLogtime[64];               		//日志时间
    char strRemark[512];                  	//备注
} EMAILSYNC_LOGINFO;

//信息日志结构体
typedef struct tagSystemLogInfo
{
    char strTime[20];                   //日志产生时间
    char strLogInfo[128];               //日志信息
} SYSTEM_LOGINFO;

//通道管理日志结构体
typedef struct tagChannelManageLogInfo
{
    int nTaskID;                  //服务ID号(系统依靠本ID号来对文件同步服务的唯一标志，因此内网与外网该ID号必须一致)
    char strTime[20];                   //操作时间
    char strLogInfo[128];               //描述(根据需求自定义)
} CHANNELMANAGE_LOGINFO;

//告警日志结构体
typedef struct tagAlarmLogInfo
{
    int nTaskID;                        //业务ID
    char strModule[32];              //所属模块(文件过滤、病毒扫描、文件同步、文件名过滤、恶意代码检测、通道、安全代理)
    char strAlarmLevel[32];             //告警级别(告警、错误)
    char strAlarmLogInfo[256];          //告警信息
    char strTime[20];                   //告警时间
    char strAlarmType[64];      //告警类型,由文件核查模块审核异常隔离：MD5完整性、关键字、病毒、文件大小限制、扩展名等
} ALARM_LOGINFO;

//文件共享日志结构体
typedef struct tagFileShareLogInfo
{
    int nId;   									//业务ID
    int nDirection; 							//内外端标识
    int nTaskID; 								//任务id
    char strUserip[64];						//用户ip
    char strUsername[64]; 					//用户名
    char strLoginfo[512]; 					//日志内容
    int nLevel; 								//涉密级别
    char strLogtime[64]; 					//日志时间
} FILESHARE_LOGINFO;

//病毒库信息结构体
typedef struct tagVirusInfo
{
    bool bEnable;                       //是否启动查杀病毒引擎
    char strVirusDate[64];              //病毒库日期信息
    char strVersion[32];                //病毒库版本信息
    char strVirusFileName[FILE_NAME_LEN]; //病毒库文件（该文件应放置在相当的病毒库目录中）
} VIRUSES_INFO;

//告警通知结构体
typedef struct tagAlarmNotify
{
    int nNotiWay;                       //告警通知方式，0所有方式，1邮件，2短信.
    int nLinkId;                        //发送源及目的id
    char strAlarmType[64];              //告警类别(CPU、内存、磁盘空间、主备切换、任务执行失败等)
    char strAlarmLevel[32];             //告警级别(告警、错误)
    char strAlarmInfo[512];             //具休的告警信息
    char strAlrmTime[20];               //告警时间
} ALARM_NOTIFYINFO;

typedef struct tagEmailCheckTemplate
{
    int nID;                            //ID号
    char strTempletName[64];            //模板名称
    bool bCheckFileExtension;           //文件扩展名核查(True:启用， false:不启用)
    char strAllowFileExtension[256];	//允许通过同步的扩展名(允许通过同步的扩展名，多个扩展名间用';'号隔开)
    bool bCheckKeyword;                 //关键字核查(True:启用， false:不启用)
    char strForbidKeyword[256];       //禁止的关键字(禁止通过的关键字，多个关键字间用';'号隔开。允许模糊查询“*”)
    bool bCheckFileSize;                //文件大小核查(True:启用， false:不启用)
    int nMinFileLen;                    //过滤最小字节数(单位：M, 大于此数值可执行同步操作)
    int nMaxFileLen;                    //过滤最大字节数(单位：Byte, 小于此数值可执行同步操作)
    bool bCheckvirus;                   //是否进行病毒查杀
    bool bStatus;						//状态 1启用  0不启用
} EMAILCHECKTEMPLATE;

typedef struct tagEmailCheckRule
{
    int nTaskID;              //任务(服务)ID号(系统依靠本ID号来对文件同步服务的唯一标志，因此内网与外网该ID号必须一致)
                              //外端服务ID号惟一，只能允许一条记录，而内端ID可以多个，对应不同的同步对像，即一对多的关系
    int nID;                                //ID号
    char RealWorkName[32];                  //实际工作目录
    char MainPath[PATH_LEN + FILE_NAME_LEN];  //实际工作目录父路径
    char strAddress[64];		    //策略对应外端的收件箱
    char strCheckRuleName[64];              //模板名称
    bool bCheckMailAddr;                    //是否检验邮件黑名单
    bool bCheckMailContent;		    //是否校验邮件内容
    bool bStatus;			    //状态
    EMAILCHECKTEMPLATE g_mailcheck_tamplate;			    //邮件过滤模板
} EMAILCHECKRULE;

//导入客户端通用配置结构体
typedef struct tagImportClientCommCfg
{
    char strLogFile[FILE_NAME_LEN];         // 日志文件（默认/var/log/importClient.log）
    int nLoglevel;                                // 日志等级（默认 1）
    int nLogRotateint;                            // 日志回滚时间（默认30天）
    int nLogRotateSize;                           // 日志回滚大小（默认100）
    int nLogRotateCount;                          // 日志回滚数量（默认6）
    int nServiceFlag;                             // 后台运行（1,后台自动运行， 0，人工干预运行）
    char strCrtFilename[FILE_NAME_LEN];     // 客户端证书文件
    char strKeyFilename[FILE_NAME_LEN];     // 客户端KEY文件
    char strCacrtFilename[FILE_NAME_LEN];         // ca文件
    char strIp[IP_LEN];                           // IP地址 （例：172.16.1.8）必填项
    int nPort;                                   // 端口 （默认8021）必填项
    char strUser[USERNAME_LEN];                   // 用户名 （例：user1）必填项
    char strPass[PASSWORD_LEN];                   // 密码 （例：user1）必填项
    int nEncrypt;                      // 传输加密 （0为不加密，1.IDEA 算法 2.RSA算法 3.AES算法）
    int nMaster;                      //双向同步时，1:是主端  0:是备端
} IMPORT_CLIENT_COMMCFG;

//导入客户端（发送端）配置结构体
typedef struct tagImportClientSendCfg
{
    char strPath[PATH_LEN];                       // 发送目录 （例：/home/user1）必填项
    char strTmpPath[PATH_LEN];                    // 临时目录 （例：/home/user1.cache）必填项
    char strVirusPath[PATH_LEN];                  //病毒目录 （例：/home/user1.cache）必填项
    char strBackup[PATH_LEN];                     // 备份目录 （为空时发送文件不需要备份）
    bool Backup;                    			 // 备份 （为0时发送文件不需要备份）
    int nWait;                                    // 文件稳定时间 （默认为3秒）
    int nBufMaxCnt;                              // 临时目录文件最大值（默认2000）
    char strTmpExten[10];                         // 临时文件后缀 （默认为.tmp）
    int nMD5Sum;                                  // MD5校验 （0为不校验，1为校验）
    int nUpEmpty;                                 // 同步空文件 （0为不同步空，1为同步）
    int nDeleteDir;                               // 删除空目录 （-1为保留1级空目录,1为删除）
//    int nSort;                                  // 排序发送 （0为不排序，1为排序）
    int nSkipFail;                                // 忽略失败文件（0为不跳过出错文件，1为跳过）
    int nIOSleep;                            	  // after send per file, need to sleep time
//    int nBlockSize;                             // 分块大小 （默认10485760Byte）
//    int nPutsCount;                             // 发送队列大小（默认100）
} IMPORT_CLIENT_SEND_CFG;

//导入客户端（接收端）配置结构体
typedef struct tagImportClientRecvCfg
{
    //int nType;                                  // 类型接收(0)
    char strPath[PATH_LEN];                       // 接收目录 （例：/home/user1）必填项
    char strTmpPath[PATH_LEN];                    // 临时目录 （例：/home/user1.cache）必填项
    char strTmpExten[10];                         // 临时文件后缀 （默认为.tmp）
    int nMD5Sum;                                  // MD5校验 （0为不校验，1为校验）
//    int nUpEmpty;                               // 同步空文件 （0为不同步空，1为同步）
//    int nDeleteDir;                             // 删除空目录 （-1为保留1级空目录,1为删除）
//    int nSort;                                  // 排序发送 （0为不排序，1为排序）
    int nSkipFail;                                // 忽略失败文件（0为不跳过出错文件，1为跳过）
//    int nBlockSize;                             // 分块大小 （默认10485760）
//    int nPutsCount;                             // 发送队列大小（默认100）
} IMPORT_CLIENT_RECV_CFG;

//以下结构为各进程间的通信协议消息及结构体定义

#define CMD_VERS_ONE    0x01    //第一版
#define CMD_VERS_TWO    0x02    //第二版

//与消息类型相关的定义
#define CMD_TYPE_SYST	0x01    //连接和协调类型
#define CMD_TYPE_TASK	0x02    //资料配置类型
#define CMD_TYPE_CTRL	0x03    //远程控制类型
#define CMD_TYPE_ALARM	0x04    //报警信息类型
#define CMD_TYPE_CHG	0x05    //状态变化类型
#define CMD_TYPE_PLAN	0x06    //方案配置类型
#define CMD_TYPE_FILE   0x07    //文件传类型(目前只用于安全导入模块与导入客户端间的文件传输)
#define CMD_TYPE_LOG    0x08    //日志类型（用于日志审计）
#define CMD_TYPE_ERROR	0x09    //系统错误类型
#define CMD_TYPE_COMM   0x0A    //通信类（主要用于传输模块的通信信息、透明传输数据(UDP及TCP代理)）
#define CMD_TYPE_ENC    0x0B    // 加密内容


//与TCP连接相关的消息定义
#define CMD_SYST_CONNECT    0x01    //连接成功
#define CMD_SYST_LOGIN      0x02    //身份验证
#define CMD_SYST_LOGERR     0x03    //登录失败
#define CMD_SYST_WELCOME    0x04    //登录成功
#define CMD_SYST_TEST       0x05    //连接测试
#define CMD_SYST_FILE_SAVE_PATH 0x06  //文件保存路径
#define CMD_SYST_ENCRYPT_REQ 0x07   //加密模式请求
#define CMD_SYST_ENCRYPT_RSP 0x08   //响应加密模式请求
#define CMD_SYST_HA_STATE   0x09    //双机热备状态信息
//fishman 加密消息
#define CMD_ENC_SIGN        0x01    //签名包
#define CMD_ENC_VERF        0x02    //验签包
#define CMD_ENC_KEYSEND     0x03    //发送会话密钥
#define CMD_ENC_KEYANS      0x04    //接收会话密钥
#define CMD_ENC_DATA        0x05    //业务数据

#define CMD_COMM_REQ  0x01   //请求
#define CMD_COMM_RSP  0x02   //应答
#define CMD_COMM_INNER_CLOSE 0x03  //内端关闭
#define CMD_COMM_OUTER_CLOSE 0x04  //外端关闭

//与任务相关的消息定义
#define CMD_TASK_GET_FILESYN_REQ  0x01          //获取文件同步信息请求
#define CMD_TASK_GET_FILESYN_RSP  0x02          //响应获取文件同步信息请求
#define CMD_TASK_NEW              0x03          //执行新任务
#define CMD_TASK_FINISHED         0x04          //任务执行完毕
#define CMD_TASK_GET_DBSRC_REQ    0x05          //获取数据库数据源配置信息请求
#define CMD_TASK_GET_DBSRC_RSP    0x06          //响应获取数据库数据源配置信息请求
#define CMD_TASK_GET_DBSYN_REQ    0x07          //获取数据库同步配置信息请求
#define CMD_TASK_GET_DBSYN_RSP    0x08          //响应获取数据库同步配置信息请求
#define CMD_TASK_CONF_CHANGE      0x10          //配置资料更改通知
#define CMD_TASK_DB_STRUCT_CHANGE 0x11          //数据库表结构更改通知
#define CMD_TASK_SAFECLIENT_TASKINFO 0x12       //导入客户端任务信息
#define CMD_TASK_RESEND_FILE_REQ  0x20          //重发文件请求
#define CMD_TASK_TEST             0x30          //内部通信测试（测试连接内外端光纤通道的通信状态）

//与文件传输相关的消息定义
#define CMD_FILE_SEND_TASK_REQ          0x01    //发送目录任务请求
#define CMD_FILE_SEND_TASK_RSP          0x02    //响应发送任务请求
#define CMD_FILE_SEND_FILE_INFO_REQ     0x03    //发送文件请求
#define CMD_FILE_SEND_FILE_INFO_RSP     0x04    //响应发送文件请求
#define CMD_FILE_SEND_FILE_DATA         0x05    //发送文件数据包
#define CMD_FILE_SEND_FILE_FINISHED_REQ 0x06    //文件发送完成包
#define CMD_FILE_SEND_FILE_FINISHED_RSP 0x07    //文件发送完成响应包
#define CMD_FILE_QUERY                  0x08    //查询服务端文件列表
#define CMD_FILE_UPLOAD_FILE_INFO_REQ   0x09    //上传文件信息  
#define CMD_FILE_UPLOAD_DATA            0x0A    //上传文件数据  
#define CMD_FILE_DOWNLOAD_FILE_INFO_REQ 0x0B    //下载文件信息  
#define CMD_FILE_DOWNLOAD_DATA          0x0C    //下载文件数据  
#define CMD_FILE_LOG_QUERY              0x0D    // 查询日志
#define CMD_FILE_FINISHED_TASK_REQ      0x0E    //发送目录任务请求
#define CMD_FILE_FINISHED_TASK_RSP      0x0F    //响应发送任务请求
#define CMD_FILE_TRANCEMODE_SET_REQ     0x10
#define CMD_FILE_TRANCEMODE_SET_RSP     0x11

//与日志审计查关的消息定义
#define CMD_LOG_LOGIN               0x01        //登录日志
#define CMD_LOG_BACKUP              0x02        //系统备份/恢复日志
#define CMD_LOG_REBOOT              0x03        //系统重启日志
#define CMD_LOG_USER_MANAGE         0x04        //用户管理日志
#define CMD_LOG_VIRUS_UPDATE        0x05        //病毒库升级日志
#define CMD_LOG_RESOURCE            0x06        //资源告警日志(CPU、内存、磁盘不足告警)
#define CMD_LOG_FILESYNC            0x07        //文件同步日志
#define CMD_LOG_DBSYNC              0x08        //数据库同步日志
#define CMD_LOG_EMAILSYNC           0x09        //邮件传输日志
#define CMD_LOG_SYSTEM              0x0A        //系统日志
#define CMD_LOG_MANAGE              0x0B        //管理日志
#define CMD_LOG_DBEXTRACT           0x0C        //数据库抽取日志
#define CMD_LOG_BASEINFO            0x0D        //基本信息日志
#define CMD_LOG_CHANNEL_MANAGE      0x0E        //通道管理日志
#define CMD_LOG_ALARM               0x0F        //告警日志
#define CMD_LOG_AGENT               0xF0        //Agent Log
#define CMD_LOG_FILESHARE           0xF1        //文件共享日志
#define CMD_LOG_WEB                 0xF2        //WEB日志
#define CMD_LOG_PROXY               0xF3        //Proxy日志
#define CMD_LOG_FTPPROXY            0xF4        //FTP代理日志
#define CMD_LOG_DBAGENT					0xF5        //数据库访问
//配置资料更改类型
typedef enum
{
    CONFTYPE_FILE,               //文件同步配置
    CONFTYPE_DB,                 //数据库同步配置
    CODETYPE_EMAIL,              //邮件同步配置
    CONFTYPE_VIRUS,              //病毒资料
    CONFTYPE_HA,                 //HA双机热备资料
    CONFTYPE_LOGSWITCH,          //日志入库启动开关
    CONFTYPE_SECRET_USER,        //涉密用户配置资料
    CONFTYPE_AGENT,              //定向代理资料配置
} CONFTYPE;

//maincode:为以上类型
//SubCode:  子代码的取值为：
//当MainCode为CONFTYPE_FILE时， 1,文件资源配置，2,文件核查配置，3，文件同步策略；
//当MainCode为CONFTYPE_DB时，   1,数据源，2,数据库同步任务，3，数据库同步策略。4，数据库同步表
//当MainCode为CONFTYPE_EMAIL时，1，
//当MainCode为CONFTYPE_VIRUS时，1，病毒查杀策略
//当MainCode为CONFTYPE_AGENT， 1，TCP定向代理配置， 2，UDP定向代理配置

//GAP包头结构
typedef struct					//定义命令头格式
{
    char HeadFlag[4];                   //包头标志为"@@@@"
    unsigned char Version;              //Version-版本号（1Byte）
    unsigned char CommandType;          //CommandType-通信类型（1Byte）
    unsigned char CommandCode;          //CommandCode-具体命令代码（1Byte）
    unsigned char ModuleType;           //模块类型(参见MODULE_TYPE 定义)
    unsigned char HeadLen;              //包头长度(其值填 sizeof(GAPPACK_HEADER))
    unsigned int TotalLength;          //TotalLength-有效数据字节长度（4Byte）包含包头+数据长度
} GAPPACK_HEADER;

//代理包头结构(跟在GAPPACK_HEADER后的数据部份的开头部份，由代理程序填充并使用)
typedef struct tagGappackAgentHeader	//定义命令头格式
{
    int OutsideSessionID;               //外端会话ID标志(由外端代理填充,可以是子进程的标记号)
    unsigned char OutsideIpVer;         //外端IP版本号(由外端代理填充) 4, IPV4 6, IPV6
    unsigned char OutsideProType;       //外端协议类型(由外端代理填充)
    int OutsideIP[4];                   //外端代理的IP地址信息(由外端代理填充 当为IPV4时,IP用OutsizeIP[0]表示，当为IPV6时，16个字节表示IPV6信息)
    int OutsidePort;                    //外端代理的端口信息(由外端代理填充)
    int InsideSessionID;                //内端会话ID标志(由内端代理填充,可以是子进程的标记号)
    unsigned char InsideIpVer;          //内端IP版本号(由内端代理填充) 4, IPV4 6, IPV6
    unsigned char InsideProType;        //内端协议类型(由内端代理填充)
    int InsideIP[4];                    //内端代理的IP地址信息(由内端代理填充 当为IPV4时,IP用OutsizeIP[0]表示，当为IPV6时，16个字节表示IPV6信息)
    int InsidePort;                     //内端代理的端口信息(由内端代理填充)
} GAPPACK_AGENT_HEADER;
typedef struct tagServerInfo
{
    int nWorkMode;    //工作模式：1 单导内端   2单导外端   3 网闸内端 4 网闸外端， 子模块根据该工作模式来启动自己的服务。
    char strOutsideInterfacePort[32];    //外端网络接口名称(eth0~eth7)
    char strOutsideServerIP[32];         //外端服务IP（指外端机用于单向传输的光模块的发送网卡IP地址）
    int nOutSidePort;                    //外端UDP服务端口（默认为9800）
    char strInnerInterfacePort[32];      //外端网络接口名称(eth0~eth7)
    char strInnerServerIP[32];           //内端服务IP（指内端机用于单向传输的光模块的接收网卡IP地址）
    int nInnerPort;                      //内端UDP接收端口（默认为9600）
    char strInnerMACAddr[32];            //内端MAC地址
} GAPSERVER_INFO;

//任务信息
typedef struct tagTaskInfo
{
//    char strSrcPath[256];               //任务所在目录
//    int nFileType;                      //文件类型
    int nTaskID;                        //任务号
    int nLevel;                         //任务级别
    char strSubPath[32];                //子目录(日期/时间)
} TASK_INFO;

//任务信息(用于安全导入模块，适用于一个源任务传给多个目的地址)
typedef struct tagTaskInfoEx
{
//    char strSrcPath[256];               //任务所在目录
//    int nFileType;                      //文件类型
    int nTaskID;                        //任务号
    int nLevel;                         //任务级别
    char strSubPath[32];                //子目录
    char strDestIP[IP_LEN];             //该任务要发送到对应的目标IP地址
    //int nDestDestPath[PATH_LEN];        //该任务要发送到对应的目标路径
} TASK_INFO_EX;

//文件核查策略结构体
typedef struct tagfile_synchrodata_server
{
    int nTaskID;              //任务(服务)ID号(系统依靠本ID号来对文件同步服务的唯一标志，因此内网与外网该ID号必须一致)
                              //外端服务ID号惟一，只能允许一条记录，而内端ID可以多个，对应不同的同步对像，即一对多的关系
    char RealWorkName[32];                  //实际工作目录
    char MainPath[PATH_LEN + FILE_NAME_LEN];  //实际工作目录父路径
                                              //外端服务ID号惟一，只能允许一条记录，而内端ID可以多个，对应不同的同步对像，即一对多的关系
    FILECHECK_TEMPLET g_FileCheck_Templet;  //文件核查模板结构体(filecheck_templet)
    VIRUS_RULE g_Virus_Rule;                //病毒查杀策略结构体(virus_rule)
} FILE_SYNCHRODATA_SERVER;

//工作目录文件夹结构体
typedef struct tagEachFileInfo
{
    char curFile[1024];                     //文件绝对路径
    long int iFileSize;                     //文件大小bytes
} EACHFILEINFO;

//email任务结构体
typedef struct tagEmainTaskInfo
{
    int taskId;                             //策略号
    char foldNmae[64];                      //工作目录&任务号
} EMAILTASKINFO;

//任务信息(用于外网传送到内网的任务信息)
typedef struct tagSyncTaskInfo
{
    int nTaskID;                            //任务ID号
    int nTaskType;                     //所属任务类型  (1, 文件同步类型，2,数据库同步类型,3, 邮件同步类型)
    int nSecretLevel;                       //涉密级别
    char strTaskName[64];                   //任务名称
} SYNC_TASK_INFO;

//安全导入客户端任务信息结构
//用于导入服务器与导入客户端之间的任务传输
typedef struct tagSafeClientTaskInfo
{
	FILESYNC_RULE rule;
    unsigned int tLastSync;             //最后一次同步时间
} SAFECLIENT_TASKINFO;

//以下为V1.1 新增的结构定义
//安全输入用户结构体
typedef struct tagSafeUserInfo
{
    char strUserName[USERNAME_LEN];         //用户名
    char strPassword[PASSWORD_LEN];         //用户密码(MD5加密)
    int nSecretLevel;                       //涉密级别(参见SECRET_LEVEL)
    int nLinkFileCheckId;                   //关联文件核查模板(关联文件核查模块表中的ID号)
    int nLinkVirusRuleId;                   //关联病毒查杀ID号(关联病毒查杀策略表中的ID号)
    int nLimitSpeed;                        //网络限速（KB)
    int nDoSameFileName;                    //文件同名处理(0,丢弃,1覆盖, 2,改名)
    int nRegUKey;                        //是否需要UKey登录验证(0,不需要Ukey验证, 1,需要Ukey验证)
} SAFE_USERINFO;

//定向传输代理
typedef struct tagTCPAgentTrans
{
    int id;                                 //ID键值
    int nTaskID;                            //任务编号
    int nSrcPort;                           //源端口号
    char strSrcAddr[IP_LEN];                //源IP地址
    char strDestAddr[IP_LEN];               //目的IP地址
    int nDestPort;                          //目的端口号
    int nProtocolType;                      //协议类型(1-tcp, 2-udp, 4-tcp6, 8-udp6)
} AGENT_TRANS;

//申请重新发送文件结构体
typedef struct tagTaskReSendFile
{
    int nSessionID;                             //文件发送会话ID号
    char strFileName[PATH_LEN + FILE_NAME_LEN];   //文件名
} TASK_RESEND_FILE;

typedef struct rescheck
{
    int nhVirus; //异常-1  无毒0  有毒1
    int nhKeyword; //异常-1  无关键字0  有关键字1
} RESCHECK;

typedef struct stratgy
{
    int nlinkid; //策略id
    int option; //预留
    int status; //状态 1启用  0不启用
    bool bcheckkeyword; //是否校验关键字
    bool bcheckvirus; //是否检验病毒
    char keyword[512]; //关键字 以|隔开
} STRATGY;

//Web日志结构
struct WebLog
{
    char time_since_epoch[32]; //请求到达的时间
    char time_response[16]; //请求和应答通过代理的时间
    char ip_client[128]; //客户端IP
    char http_status[64]; //http应答状态
    char http_reply_size[16];
    char http_method[32]; //http方法
    char http_url[128];
    char http_username[32];
    char ip_server[128]; //web服务器ip
    char http_mime_type[64];
};

//代理日志结构
typedef struct
{
    unsigned int taskid;   // 任务号
    unsigned int direction; //标识
    char sjtime[32];   // 日志时间
    char message[256]; // 日志内容
} ProxyLog;

//数据库访问日志
typedef struct
{
    int direction;
    int taskid;
    char loginfo[512];
    int tid;
    char logtime[32];
} DBAGENTLOG;

#pragma  pack()
#endif

