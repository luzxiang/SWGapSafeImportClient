/*
 * FIFO_.h
 *
 *  Created on: 2018年9月12日
 *      Author: luzxiang
 */

#ifndef FIFO_FIFO_H_
#define FIFO_FIFO_H_
#include <mutex>
#ifdef OS_LINUX
#include <sys/time.h>
#endif
#include <thread>
#include <chrono>
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>    // std::condition_variable
#include <string.h>
#include "FIFO_.h"
using namespace std;

typedef std::unique_lock <std::mutex> mtxlock;
typedef struct sockt_fifo{
	FIFO_ *Buf;
	std::mutex mtx; // 全局互斥锁.
	std::condition_variable cv; // 全局条件变量.
//    pthread_mutex_t mtx;
//    pthread_cond_t cv;
    //数据写入缓冲区
    unsigned int Put(const char *buf, unsigned int len)
    {/* notice: if you have just one thread to call Push function,
    	 * necessary to enable the thread lock here, else you must enable*/
//    	lock();
    	mtxlock lck(mtx);
    	len = Buf->Put(buf, len);
//    	pthread_cond_signal(&this->cv);
    	this->cv.notify_all(); // 唤醒所有线程.
//    	unlock();
    	return len;
    }
    void Release(void)
    {
    }
//    void lock(void)
//    {
//    	pthread_mutex_lock(&this->mtx);
//    }
//    unsigned int timewait(unsigned int tout_s)
//    {
//    	timewait(tout_s, 0);
//    	return 0;
//    }
//    unsigned int timewait(long int tout_s, long int tout_msec)
//    {
//    	struct timeval now;
//		gettimeofday(&now, NULL);
//		now.tv_usec += tout_msec * 1000;//us
//		now.tv_sec += now.tv_usec / 1000000;//s
//		now.tv_usec %= 1000000;
//
//    	struct timespec tout;
//    	memset(&tout, 0, sizeof(tout));
//    	tout.tv_nsec = now.tv_usec*1000;
//    	tout.tv_sec = now.tv_sec + tout_s;
//    	pthread_cond_timedwait(&this->cv, &this->mtx,&tout);
//    	return 0;
//    }
//    void unlock(void)
//    {
//    	pthread_mutex_unlock(&this->mtx);
//    }
    //从缓冲给读取数据
    unsigned int Get(char *buf, unsigned int len)
    {
    	return Buf->Get(buf, len);
    }
    unsigned int HaveFree(void) const
    {
    	return Buf->size - Buf->Len();
    }
    bool HaveFree(unsigned int size) const
    {
    	return (Buf->size - Buf->Len() >= size);
    }
    //返回缓冲区中数据长度
    unsigned int Len() const{
    	return Buf->Len();
    }
    bool Empty() const
    {
    	return Buf->Empty();
    }
	sockt_fifo(unsigned int size = 1024)
	{
		Buf = new FIFO_(size);
//	    mtx = PTHREAD_MUTEX_INITIALIZER;
//	    cv = PTHREAD_COND_INITIALIZER;
	}
	sockt_fifo(char *buf, unsigned int size)
	{
		Buf = new FIFO_(buf, size);
//	    mtx = PTHREAD_MUTEX_INITIALIZER;
//	    cv = PTHREAD_COND_INITIALIZER;
	}
	virtual ~sockt_fifo()
	{
//		pthread_cond_destroy(&this->cv);
//		pthread_mutex_destroy(&this->mtx);
	}
}sockt_fifo_st;



#endif /* FIFO_FIFO__H_ */
