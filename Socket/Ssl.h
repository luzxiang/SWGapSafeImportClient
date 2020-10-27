/*
 * Ssl.h
 *
 *  Created on: 2018年9月4日
 *      Author: luzxiang
 */

#ifndef SOCKET_SSL_H_
#define SOCKET_SSL_H_

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <iostream>
using namespace std;

class Ssl{
private:
	SSL *m_pSsl;                        //SSL 对象指针
	SSL_METHOD *m_pMeth;                //SSL方法指针
	SSL_CTX *m_pCtx;                    //SSL ctx指针
public :
	string  m_KeyFilename;//[1024];
	string m_CacrtFilename;//[1024];
	string m_CrtFilename;//[1024];
	Ssl(void);
	virtual ~Ssl(void);
	Ssl(char*key, char* cacrt, char*crt);
	int Init(char*key, char* cacrt, char*crt);
	bool Set_fd(int fd);
	int Close(void);
	int	Send(const void *buf,int num);
	int Read(void *buf,int num);
};


#endif /* SOCKET_SSL_H_ */
