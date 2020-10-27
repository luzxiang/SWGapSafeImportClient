/*
 * Socket.h
 *
 *  Created on: 2018年9月10日
 *      Author: luzxiang
 */

#ifndef SOCKET_SOCKET_H_
#define SOCKET_SOCKET_H_
#include "../FM/UKey.h"
#include "../com/swgap_comm.h"
#include "ISocket.h" 
#ifdef OS_WIN 
#include <WinSock2.h>
#include <atlstr.h> 
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#endif
class Socket:public ISocket
{
public :
    UKey *ukey;
    Ssl *ssl;
	int Encrypt;        //传输加密模式 0 不加密 1:ssl加密 2:guomi
	~Socket(void);
	Socket(void);
	Socket(char*ip, int port);
	void SetSockOpt(void);
	void Close(void);

	int Connect(void);
    int WaitWrite(const char *buf, const size_t len);
    void SetEncrypt(int eypt);

	bool GetHeadPackValue(unsigned char nCmdType, unsigned char nCmdCode, unsigned int nDataLength, GAPPACK_HEADER &PackHead);
	int ukey_GetSymEncryKey(void);
	void ukey_GetSymKeyData(char *buf, int &lenbuf);
	int ukey_SymDecrypt(const char *s_in, int len_in,
	                         char *s_out, int &len_out);
	void ukey_GetSignData(char *buf, int &lenpag);

};



#endif /* SOCKET_SOCKET_H_ */
