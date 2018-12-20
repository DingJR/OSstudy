/*************************************************************************
    > File Name: spinlock.h
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Tue 20 Nov 2018 09:30:33 PM CST
 ************************************************************************/
#ifndef _SPINLOCK_H
#define _SPINLOCK_H
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
//#define TICKET

#ifdef TICKET
typedef struct spinlock_t
{
	unsigned user;
	unsigned ticket;
//unsigned short spin;
}spinlock_t;
#else
typedef unsigned short spinlock_t;
#endif

typedef struct spinlock_t* spinlock;
void spinlock_init(spinlock_t* inLock);
void spinlock_acquire(spinlock_t* inLock);
void spinlock_release(spinlock_t* inLock);

#endif
