/*
 * config.cpp
 *
 *  Created on: 2018年8月28日
 *      Author: root
 */
#include <stdlib.h>
#ifdef OS_LINUX
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "./config.h"
#include "../log/log.h"  
#include "../com/os_app.h"  

string Config::exepath = "";
bool Config::isSetted = false;
IMPORT_CLIENT_COMMCFG Config::Com;     //通用部份配置信息
IMPORT_CLIENT_SEND_CFG Config::Sender;    //发送端使用的配置信息
IMPORT_CLIENT_RECV_CFG Config::Recver;    //接收端使用的配置信息
GAPSERVER_INFO Config::SvrInfo;

Config::Config(void)
{
	Reset();
}
/*******************************************************************************
 * Function     : Config::reset
 * Description  : reset
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Config::Reset(void) {
	memset(&Com,0,sizeof(Com));
	memset(&Sender,0,sizeof(Sender));
	memset(&Recver,0,sizeof(Recver));
	memset(&SvrInfo,0,sizeof(SvrInfo));
}
/*******************************************************************************
 * Function     : GetValueByKey
 * Description  : 从INI文件读取字符串类型数据
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
char *Config::GetValueByKey(const char *key, const char *value, const char *fromfile)
{
    if (NULL == key || NULL == value || NULL == fromfile)
        return NULL;
    FILE *fp = NULL;
    char szLine[1024] = { 0 };
    static char tmpstr[1024] = { 0 };
    int rtnval = 0;
    int i = 0;
    int flag = 0;
    char *tmp = NULL;
    if ((fp = fopen(fromfile, "rb")) == NULL)
    {
        printf("have no such file:%s \n", fromfile);
        return tmpstr;
    }
    while (!feof(fp))
    {
        rtnval = fgetc(fp);
        if (rtnval == EOF)
        {
            break;
        }
        else
        {
            szLine[i++] = rtnval;
        }
        if (rtnval == '\n')
        {
            szLine[--i] = '\0';
            i = 0;
            tmp = strchr(szLine, '=');

            if ((tmp != NULL) && (flag == 1))
            {
                if (strstr(szLine, value) != NULL)
                {
                    //注释行
                    if ('#' == szLine[0])
                    {
                        continue;       //继续到下一行
                    }
                    else if ('/' == szLine[0] && '/' == szLine[1])
                    {
                        continue;       //继续到下一行
                    }
                    else if (strncmp(szLine, value, strlen(value)) != 0)
                    {
                        continue;
                    }
                    else
                    {
                        //找到key对应变量
                        strcpy(tmpstr, tmp + 1);
                        fclose(fp);
                        return tmpstr;
                    }
                }
            }
            else
            {
                strcpy(tmpstr, "[");
                strcat(tmpstr, key);
                strcat(tmpstr, "]");
                if (strncmp(tmpstr, szLine, strlen(tmpstr)) == 0)
                {
                    //找到title
                    flag = 1;
                }
            }
        }
    }
    fclose(fp);
    strcpy(tmpstr, "0");    //返回默认值
    return tmpstr;
}
/*******************************************************************************
 * Function     : Config::GetExePath
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
std::string Config::GetExePath(void){
	if(!isSetted)
	{
		SetExePath();
	}
	return exepath;
}
/*******************************************************************************
 * Function     : GetCurrentPath
 * Description  : GetCurrentPath
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int Config::SetExePath(void)
{
    char path[512] = {0};
#ifdef OS_LINUX 
    if (readlink("/proc/self/exe", path, sizeof(path)) <= 0)
        return -2;
    char* path_end = strrchr(path, '/');
#endif 
#ifdef OS_WIN 
	GetModuleFileName(NULL, path, MAX_PATH);
    char* path_end = strrchr(path, '\\'); 
#endif 
    if (path_end == NULL)
        return -2;
    ++path_end;
    *path_end = '\0';
	exepath = os_format2dir(path,sizeof(path));
    isSetted = true;
    return exepath.length();
}
/*******************************************************************************
 * Function     : GetCurrentPath
 * Description  : GetCurrentPath
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int Config::SetExePath(char* argv)
{ 
	char *path = strdup(argv);
#ifdef OS_LINUX
    char* path_end = strrchr(path, '/');
#endif
#ifdef OS_WIN 
    char* path_end = strrchr(path, '\\'); 
#endif 
    if (path_end == NULL)
        return -2;
    ++path_end;
    *path_end = '\0'; 
    exepath = path;
    isSetted = true;
	if(path) 
		free(path);
	path = NULL;
    return exepath.length();
}
/*******************************************************************************
 * Function     : GetCfgInfo
 * Description  : 从本地配置文件中获取配置信息
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int Config::Set(char* argv)
{
	char ini[50] = INIFILE;
	os_format2path(ini,sizeof(ini));
	int rst = 0;
    Reset();
	SetExePath(argv);
    std::string inifile = GetExePath() + ini;
    //以下为通用部分
    Com.nLoglevel = atoi(GetValueByKey("COMMON", "logLevel", inifile.c_str()));
    Com.nLogRotateint = atoi(GetValueByKey("COMMON", "logRotateInt", inifile.c_str()));
    Com.nLogRotateSize = atoi(GetValueByKey("COMMON", "logRotateSize", inifile.c_str()));
    Com.nLogRotateCount = atoi(GetValueByKey("COMMON", "logRotateCount", inifile.c_str()));
    Com.nServiceFlag = atoi(GetValueByKey("COMMON", "service", inifile.c_str()));
    Com.nPort = atoi(GetValueByKey("COMMON", "port", inifile.c_str()));
    Com.nEncrypt = atoi(GetValueByKey("COMMON", "encrypt", inifile.c_str()));
    Com.nMaster = atoi(GetValueByKey("COMMON", "master", inifile.c_str()));
	
//    OS_SNPRINTF(Com.strLogFile, sizeof(Com.strLogFile), "%s", GetValueByKey("COMMON", "logfile", inifile.c_str()));
//    OS_SNPRINTF(Com.strCrtFilename, sizeof(Com.strCrtFilename), "%s",
//            GetValueByKey("COMMON", "crtFile", inifile.c_str()));
//    OS_SNPRINTF(Com.strKeyFilename, sizeof(Com.strKeyFilename), "%s",
//            GetValueByKey("COMMON", "keyFile", inifile.c_str()));
//    OS_SNPRINTF(Com.strCacrtFilename, sizeof(Com.strCacrtFilename), "%s",
//            GetValueByKey("COMMON", "cacrtFile", inifile.c_str()));
    OS_SNPRINTF(Com.strIp, sizeof(Com.strIp), "%s", GetValueByKey("COMMON", "ip", inifile.c_str()));
    OS_SNPRINTF(Com.strUser, sizeof(Com.strUser), "%s", GetValueByKey("COMMON", "user", inifile.c_str()));
    OS_SNPRINTF(Com.strPass, sizeof(Com.strPass), "%s", GetValueByKey("COMMON", "pass", inifile.c_str()));
	
    std::string cafile;

	strcpy(ini, "certs/proxy.crt");
    cafile = GetExePath() + ini;
    OS_SNPRINTF(Com.strCrtFilename, sizeof(Com.strCrtFilename), "%s", cafile.c_str());

	strcpy(ini, "certs/proxy.key");
    cafile = GetExePath() + ini;
    OS_SNPRINTF(Com.strKeyFilename, sizeof(Com.strKeyFilename), "%s", cafile.c_str());
	
	strcpy(ini, "certs/ca.crt");
    cafile = GetExePath() + ini;
    OS_SNPRINTF(Com.strCacrtFilename, sizeof(Com.strCacrtFilename), "%s", cafile.c_str());
	
    //以下部份为发送端的配置信息
    Sender.nMD5Sum = atoi(GetValueByKey("SENDER", "md5sum", inifile.c_str()));
    Sender.nWait = atoi(GetValueByKey("SENDER", "wait", inifile.c_str()));
    Sender.nUpEmpty = atoi(GetValueByKey("SENDER", "upEmpty", inifile.c_str()));
    Sender.nDeleteDir = atoi(GetValueByKey("SENDER", "delDir", inifile.c_str()));
    Sender.nSkipFail = atoi(GetValueByKey("SENDER", "skipFail", inifile.c_str()));
    Sender.nBufMaxCnt = atoi(GetValueByKey("SENDER", "bufMax", inifile.c_str()));
	Sender.nIOSleep = atoi(GetValueByKey("SENDER", "IOSleep", inifile.c_str()));

	OS_SNPRINTF(Sender.strPath, sizeof(Sender.strPath), "%s",
            GetValueByKey("SENDER", "path", inifile.c_str()));
    OS_SNPRINTF(Sender.strBackup, sizeof(Sender.strBackup), "%s",
            GetValueByKey("SENDER", "backup", inifile.c_str()));
    OS_SNPRINTF(Sender.strTmpPath, sizeof(Sender.strTmpPath), "%s",
            GetValueByKey("SENDER", "tempPath", inifile.c_str()));
    OS_SNPRINTF(Sender.strVirusPath, sizeof(Sender.strVirusPath), "%s",
            GetValueByKey("SENDER", "VirusPath", inifile.c_str())); 
    OS_SNPRINTF(Sender.strTmpExten, sizeof(Sender.strTmpExten), "%s",
            GetValueByKey("SENDER", "tmp", inifile.c_str())); 

	//对配置文件的的参数进行安全保护，以防用户设置为不合理参数，仅作为限制用户可设范围
    if (Sender.nBufMaxCnt < FILES_BUFSIZE_MIN)
        Sender.nBufMaxCnt = FILES_BUFSIZE_MIN;    //限制最小值
    else if (Sender.nBufMaxCnt > FILES_BUFSIZE_MAX)
        Sender.nBufMaxCnt = FILES_BUFSIZE_MAX;    //限制最大值 

	if (Sender.nIOSleep < 0)
		Sender.nIOSleep = 0;

	os_format2abpath(Com.strCrtFilename,sizeof(Com.strCrtFilename));
	os_format2abpath(Com.strKeyFilename,sizeof(Com.strKeyFilename));
	os_format2abpath(Com.strCacrtFilename,sizeof(Com.strCacrtFilename));

	os_format2dir(Sender.strPath,sizeof(Sender.strPath));
	os_format2dir(Sender.strBackup,sizeof(Sender.strBackup));
	os_format2dir(Sender.strTmpPath,sizeof(Sender.strTmpPath));
	os_format2dir(Sender.strVirusPath,sizeof(Sender.strVirusPath));
	os_format2dir(Sender.strTmpExten,sizeof(Sender.strTmpExten));

    Sender.Backup = (strlen(Sender.strBackup) > 2)? (true):(false);

	if(Sender.Backup)
	{
		if(CheckPathValid(Sender.strBackup) < 0)
			rst = -1;
		LOG_ERROR("Sender.strBackup is invalid");
	}
	if(CheckPathValid(Sender.strTmpPath) < 0)
	{
		rst = -1;
		LOG_ERROR("Sender.strTmpPath is invalid");
	}
    //以下为接收端部分
    Recver.nMD5Sum = atoi(GetValueByKey("RECEIVE", "md5sum", inifile.c_str()));
    Recver.nSkipFail = atoi(GetValueByKey("RECEIVE", "skipFail", inifile.c_str()));

    OS_SNPRINTF(Recver.strPath, sizeof(Recver.strPath), "%s", 
		GetValueByKey("RECEIVE", "path", inifile.c_str()));
    OS_SNPRINTF(Recver.strTmpPath, sizeof(Recver.strTmpPath), "%s",
		GetValueByKey("RECEIVE", "tempPath", inifile.c_str())); 
    OS_SNPRINTF(Recver.strTmpExten, sizeof(Recver.strTmpExten), "%s",
		GetValueByKey("RECEIVE", "tmp", inifile.c_str())); 
	
    os_format2dir(Recver.strPath,sizeof(Recver.strPath));
    os_format2dir(Recver.strTmpPath,sizeof(Recver.strTmpPath));
    os_format2dir(Recver.strTmpExten,sizeof(Recver.strTmpExten));

    LOG_WATCH(Com.nPort);
    LOG_WATCH(Com.nEncrypt);
    LOG_WATCH(Com.nMaster);
    LOG_WATCH(Com.strIp);
    LOG_WATCH(Com.strUser);
    LOG_WATCH(Com.strPass);
    LOG_WATCH(Com.strCrtFilename);
    LOG_WATCH(Com.strKeyFilename);
    LOG_WATCH(Com.strCacrtFilename);

    LOG_WATCH(Sender.nBufMaxCnt);
    LOG_WATCH(Sender.nIOSleep);
    LOG_WATCH(Sender.strTmpPath);
    LOG_WATCH(Sender.strBackup);
    LOG_WATCH(Sender.strVirusPath);
	
	return rst;
}
