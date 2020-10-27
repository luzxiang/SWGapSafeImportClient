/*
 * Socket.cpp
 *
 *  Created on: 2018年9月10日
 *      Author: luzxiang
 */
#ifdef OS_LINUX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#endif
#include <chrono>
#include <thread>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <sys/types.h> 
#include "Socket.h"
#include "../log/log.h"

/*******************************************************************************
 * Function     : Socket::Socket
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
Socket::Socket(void) : ISocket()
{
	ukey = NULL;
	ssl = NULL;
    Encrypt = 0;
} 
/*******************************************************************************
 * Function     : Socket::Socket
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
Socket::Socket(char*ip, int port):ISocket(ip, port)
{
	ukey = NULL;
	ssl = new Ssl();
    Encrypt = 0;
}
/*******************************************************************************
 * Function     : Socket::Socket
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
Socket::~Socket(void)
{
	Release();
	if(ssl != nullptr)
	{
		ssl->Close();
		delete ssl;
		ssl = nullptr;
	}
	if(ukey != nullptr)
	{
		ukey->Close();
		delete ukey;
		ukey = nullptr;
	}
}
/*******************************************************************************
 * Function     : SetSockOpt
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes       : --
 *******************************************************************************/
void Socket::SetSockOpt(void)
{
#ifdef OS_LINUX
    int b_on = 0;       //设置为阻塞模式
    ioctl(fd, FIONBIO, &b_on);

	int reuse0=1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse0 , sizeof(reuse0));

    int rLen = 8*1024 * 1024;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*) &rLen, sizeof(rLen));
    //发送缓冲区
    int wLen = 8*1024 * 1024;
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*) &wLen, sizeof(wLen));
    int tout = 5*1000; //5秒
    //发送时限
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &tout, sizeof(tout));
    //接收时限
    tout = 5*1000; //5秒
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &tout, sizeof(tout));
    // 禁用Nagle算法
    int nodelay = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
#endif

#ifdef OS_WIN
	u_long nVal= 0;
	int nRet = ioctlsocket(fd, FIONBIO, &nVal);
	int nRecvBuf=8*1024 * 1024;
	setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
	//发送缓冲区
	int nSendBuf=8*1024 * 1024;
	setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
	int nNetTimeout = 5000;//
	//发送时限
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, ( char * )&nNetTimeout, sizeof( int ));
	nNetTimeout = 10*1000;//
	//接收时限
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, ( char * )&nNetTimeout, sizeof( int ));
#endif
}
/*******************************************************************************
 * Function     : Close
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Socket::Close(void)
{
    IsConnected = false;
    SLEEP_MS(300);
    if (fd > 0)
	{
#ifdef OS_LINUX
		close(fd); 
#endif 
#ifdef OS_WIN
	closesocket(fd); 
	WSACleanup();
#endif
		fd = -1;
	}
	if(ssl != nullptr)
	{
		ssl->Close();
	}
	if(ukey != nullptr)
	{
		ukey->Close();
	}
	Read = std::bind(&recv, _1, _2, _3, _4);
	Send = std::bind(&send, _1, _2, _3, _4);
}

/*******************************************************************************
 * Function     : Socket::GetHeadPackValue
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
bool Socket::GetHeadPackValue(unsigned char nCmdType, unsigned char nCmdCode, unsigned int nDataLength,
        GAPPACK_HEADER &PackHead)
{
    strcpy(PackHead.HeadFlag, "####");
    PackHead.Version = CMD_VERS_ONE;
    PackHead.HeadLen = sizeof(GAPPACK_HEADER);
    PackHead.ModuleType = MODTYPE_IMPORT_CLIENT;
    PackHead.CommandType = nCmdType;
    PackHead.CommandCode = nCmdCode;
    PackHead.TotalLength = sizeof(GAPPACK_HEADER) + nDataLength;
    return true;
}

/*******************************************************************************
 * Function     : Connect
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --Encrypt
 *******************************************************************************/
int Socket::Connect(void)
{
#ifdef OS_WIN
	//WIN 首先要被始化SOCKET环境
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
	{
		LOG_INFO("WSAStartup()fail:%d\n",GetLastError());
		return -1;
	}
	memset(&SvrAddr,0, sizeof(SvrAddr)); 
#endif 
#ifdef OS_LINUX
	bzero( &SvrAddr , sizeof( SvrAddr ) ); 
#endif 
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(Port);

    if (inet_pton(SvrAddr.sin_family, Ip, &SvrAddr.sin_addr) == 0)
    {
        LOG_WARN("Invalid address.");
        return - 1;
    }
	fd = socket(SvrAddr.sin_family, SOCK_STREAM, 0);
    if (fd <= 0)
	{
    	LOG_WARN("socket error: %s",strerror(errno));
		return -1;
	}
	else
	{
		LOG_INFO("socket created successful,fd=%d",fd);
	}
	// 设置属性
    SetSockOpt();
    if(connect(fd, (struct sockaddr*) &SvrAddr, sizeof(SvrAddr)) == -1)
	{
		LOG_WARN("connect error:%s", strerror(errno));
		return -1;
	}
	LOG_INFO("connect to server:%s successful!!!\n",this->Ip);
    if(ssl->Set_fd(fd))
    {
    	LOG_INFO("ssl set successful !!!\n");
    	this->IsConnected = true;
    }
	return 0;
}

/*******************************************************************************
 * Function     : Socket::SetEncrypt
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Socket::SetEncrypt(int eypt)
{
	this->Encrypt = eypt;
    if (Encrypt == 0 || Encrypt == 2)
    {
		Read = std::bind(&recv, _1, _2, _3, _4);
		Send = std::bind(&send, _1, _2, _3, _4);
	}
    else if (Encrypt == 1)
    {
        this->Read = std::bind(&Ssl::Read, this->ssl, _2, _3);
        this->Send = std::bind(&Ssl::Send, this->ssl, _2, _3);
	}
}
/*******************************************************************************
 * Function     : Socket::Send
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int Socket::WaitWrite(const char *buf, const size_t len)
{
	if ( buf == NULL)
		return -1;
	char *data = (char *)buf;
	int data_len = len;

	if (this->Encrypt == 2)
	{
		char *cipher = NULL;
		int cipher_len = len;
		size_t padd_num = 16 - len % 16;
		if (padd_num != 16)
			cipher_len = len + padd_num;
		cipher = new char[sizeof(GAPPACK_HEADER) + cipher_len];
		// 加密数据
		LOG_TRACE("开始加密[%d]字节", len);
		if (ukey->SymEncrypt(buf, len, cipher + sizeof(GAPPACK_HEADER), cipher_len))
		{
			LOG_ERROR("加密失败");
			delete[] cipher;
			return -1;
		}
		LOG_TRACE("加密[%d]字节结束", len);
		// 封装包头
		GAPPACK_HEADER *head = (GAPPACK_HEADER*) cipher;
		this->GetHeadPackValue(CMD_TYPE_ENC, CMD_ENC_DATA, cipher_len, *head);
		LOG_TRACE("GetHeadPackValue结束,%d",cipher_len);

		data = cipher;
		data_len = sizeof(GAPPACK_HEADER) + cipher_len;
		delete[] cipher;
	}
    int slen = this->wPut(data,data_len,60*3,0);

    if(slen <= 0)
    {
    	LOG_WARN("网络超时，发送失败");
    }
    return slen;
}
/*******************************************************************************
 * Function     : Socket::ukey_GetSymEncryKey
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int Socket::ukey_GetSymEncryKey(void)
{
	return this->ukey->GetSymEncryKey();
}
/*******************************************************************************
 * Function     : Socket::ukey_GetSymKeyData
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Socket::ukey_GetSymKeyData(char *buf, int &buflen)
{
	return this->ukey->GetSymKeyData(buf, buflen);
}
/*******************************************************************************
 * Function     : Socket::ukey_SymDecrypt
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int Socket::ukey_SymDecrypt(const char *s_in, int len_in,
                         char *s_out, int &len_out)
{
	return this->ukey->SymDecrypt(s_in, len_in, s_out, len_out);
}
/*******************************************************************************
 * Function     : Socket::ukey_GetSignData
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
void Socket::ukey_GetSignData(char *buf, int &lenpag)
{
	return this->ukey->GetSignData(buf, lenpag);
}


