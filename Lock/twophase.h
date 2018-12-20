/*************************************************************************
    > File Name: twophase.h
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Thu 22 Nov 2018 10:38:41 PM CST
 ************************************************************************/

#ifndef _TWOPHASE_H
#define _TWOPHASE_H
#include "spinlock.h"
#include "mutex.h"
typedef int twophase_t;
void twophase_init(twophase_t *mylock);
void twophase_acquire(twophase_t *mylock);
void twophase_release(twophase_t *mylock);
#endif
