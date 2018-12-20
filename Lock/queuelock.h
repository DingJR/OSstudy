/*************************************************************************
    > File Name: spinlock.h
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Tue 20 Nov 2018 09:30:33 PM CST
 ************************************************************************/
#ifndef _SPINLOCK_H
#define _SPINLOCK_H
#include "myque.h"
//#define TICKET
typedef struct spinlock_t
{
	lqueue_t *q;
	int flag;
	int count;
	int guard;
}spinlock_t;
void spinlock_init(spinlock_t* inLock);
void spinlock_acquire(spinlock_t* inLock);
void spinlock_release(spinlock_t* inLock);

#endif
