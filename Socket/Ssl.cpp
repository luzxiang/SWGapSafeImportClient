/*
 * Ssl.cpp
 *
 *  Created on: 2018年9月4日
 *      Author: luzxiang
 */

#ifndef SOCKET_SSL_CPP_
#define SOCKET_SSL_CPP_

#include <iostream>
#include <stdio.h> 
#include <string.h>
#include "Ssl.h"
#include "../log/log.h"
using namespace std;
/*******************************************************************************
 * Function     : Ssl::Ssl
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
Ssl::Ssl()
{
	m_pSsl = nullptr;                        //SSL 对象指针
	m_pMeth = nullptr;                //SSL方法指针
	m_pCtx = nullptr;                    //SSL ctx指针 
	m_KeyFilename = "";
	m_CacrtFilename = "";
	m_CrtFilename = "";
}
/*******************************************************************************
 * Function     : Ssl::Ssl
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
Ssl::Ssl(char*key, char* cacrt, char*crt)
{
	m_pSsl = nullptr;                        //SSL 对象指针
	m_pMeth = nullptr;                //SSL方法指针
	m_pCtx = nullptr;                    //SSL ctx指针

	Init(key, cacrt, crt);
}
/*******************************************************************************
 * Function     : Ssl::~Ssl
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
Ssl::~Ssl(void)
{
	Close();
}
/*******************************************************************************
 * Function     : Ssl::Init
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int Ssl::Init(char*key, char* cacrt, char*crt)
{
	int rst = 0;
	LOG_WATCH(key);
	m_KeyFilename = key;
	LOG_WATCH(cacrt);
	m_CacrtFilename = cacrt; 
	LOG_WATCH(crt);
	m_CrtFilename = crt; 
	return rst;
}
/*******************************************************************************
 * Function     : Ssl::Set_fd
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
bool Ssl::Set_fd(int fd)
{
	if(m_KeyFilename.empty() || m_CacrtFilename.empty() || m_CrtFilename.empty())
	{
		LOG_ERROR("ssl is do not init, check please!!!");
		return false;
	}
    OpenSSL_add_ssl_algorithms ();
    SSL_load_error_strings();
    m_pMeth = (SSL_METHOD *) TLSv1_client_method();
    m_pCtx = SSL_CTX_new(m_pMeth);
    if (NULL == m_pCtx)
    {
        LOG_ERROR("SSL_CTX_new  return NULL");
    }
    SSL_CTX_set_verify(m_pCtx, SSL_VERIFY_PEER, NULL);
    if (0 == SSL_CTX_load_verify_locations(m_pCtx, /*CACERT*/m_CacrtFilename.c_str(), NULL))
    {
        LOG_ERROR("%s", m_CacrtFilename.c_str());
        ERR_print_errors_fp(stderr);
    }
    if (0 == SSL_CTX_use_certificate_file(m_pCtx, m_CrtFilename.c_str(), SSL_FILETYPE_PEM))
    {
        ERR_print_errors_fp(stderr);
    }
    if (0 == SSL_CTX_use_PrivateKey_file(m_pCtx, m_KeyFilename.c_str()/*MYKEYF*/, SSL_FILETYPE_PEM))
    {
        ERR_print_errors_fp(stderr);
    }
    if (!SSL_CTX_check_private_key(m_pCtx))
    {
        LOG_ERROR("Private key does not match the certificate public key");
    }
    int seed_int[100];
    int i = 0;
    srand((unsigned) time(NULL));
    for (i = 0; i < 100; i++)
        seed_int[i] = rand();
    RAND_seed(seed_int, sizeof(seed_int));
    SSL_CTX_set_cipher_list(m_pCtx, "RC4-MD5");
    SSL_CTX_set_mode(m_pCtx, SSL_MODE_AUTO_RETRY);
    m_pSsl = SSL_new(m_pCtx);
    if (NULL == m_pSsl)
    {
        LOG_ERROR("SSL_new  return NULL");
    }
    if (0 >= SSL_set_fd(m_pSsl, fd))
    {
        LOG_ERROR("Attach to Line fail!");
    }
    int k = SSL_connect(m_pSsl);
    if (0 == k)
    {
        LOG_ERROR("SSL connect fail %d!", k);
    }
    return true;
}
/*******************************************************************************
 * Function     : Ssl::Close
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int Ssl::Close(void)
{
	if(m_pMeth){
//		delete m_pMeth;//not use delete
		m_pMeth = nullptr;
	}
	if(m_pCtx) {
        SSL_CTX_free(m_pCtx);
        m_pCtx = nullptr;
	}
    if (m_pSsl)
    {
        SSL_shutdown(m_pSsl);
        SSL_free(m_pSsl);
        m_pSsl = nullptr;
    }
	return 0;
}
/*******************************************************************************
 * Function     : Write
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int	Ssl::Send(const void *buf,int num)
{
	if(this->m_pSsl)
		return SSL_write(this->m_pSsl, buf, num);
	else
	{
		LOG_WARN("%s","this->m_pSsl == NULL!!!");
		return 0;
	}
}
/*******************************************************************************
 * Function     : Read
 * Description  : TODO
 * Input        :
 * Return       :
 * Author       : luzx
 * Notes        : --
 *******************************************************************************/
int Ssl::Read(void *buf,int num)
{
	if(this->m_pSsl)
		return SSL_read(this->m_pSsl, buf, num);
	else
		return 0;
}

#endif /* SOCKET_SSL_CPP_ */
