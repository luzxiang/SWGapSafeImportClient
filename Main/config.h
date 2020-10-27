/*
 * config.h
 *
 *  Created on: 2018年8月28日
 *      Author: root
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <iostream>
#include "../com/swgap_comm.h"
using namespace std;

#define FILES_BUFSIZE_MIN   (1)//g_nFiles当前已扫描到可以发送的文件数量最小值
#define FILES_BUFSIZE_MAX   (10000)//g_nFiles当前已扫描到可以发送的文件数量最大值
#define INIFILE "bdGapsafeImport.ini"

class Config{
private:
public:
	static string exepath;
	static bool isSetted;
	static IMPORT_CLIENT_COMMCFG Com;     //通用部份配置信息
	static IMPORT_CLIENT_SEND_CFG Sender;    //发送端使用的配置信息
	static IMPORT_CLIENT_RECV_CFG Recver;    //接收端使用的配置信息
	static GAPSERVER_INFO SvrInfo;
private:
	Config();
	static int SetExePath(void);
	static int SetExePath(char* argv);
	static char *GetValueByKey(const char *key, const char *value, const char *fromfile);
public:
	static  string GetExePath(void);
	static void Reset(void); 
	static int Set(char* dir);
};
#endif /* CONFIG_H_ */







