/*************************************************************************
    > File Name: spinlock.c
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Wed 21 Nov 2018 06:29:48 PM CST
 ************************************************************************/
#include <linux/sched.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/sysinfo.h>
#include<sys/wait.h>
#include<unistd.h>
#include <pthread.h>

#define __USE_GNU
#include <unistd.h>
#include <linux/fcntl.h>
#include<ctype.h>
#include<string.h>

#include <malloc.h>
#include <sys/time.h>
#include "myque.h"
#include "spinlock.h"
#include "futex_wrapper.h"
#define cmpxchg(P,O,N) __sync_val_compare_and_swap((P),(O),(N))
#define ATMXADD(P,V) __sync_fetch_and_add((P),(V))
#define BARRIER    asm("":::"memory")
#define SYSPAUSE   asm("pause":::"memory")
uint xchg(volatile int *addr, unsigned int newval)
{
    uint result;
    asm volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
    return result;
}
void spinlock_init(spinlock_t* inLock)
{
	inLock->flag = 0;
	inLock->guard = 0;
	inLock->q = (lqueue_t *)malloc(sizeof(lqueue_t));
	queue_init(inLock->q);
}
void spinlock_acquire(spinlock_t* inLock)
{
	while(xchg(&inLock->guard,1) == 1);

		if(__sync_bool_compare_and_swap(&inLock->flag,0,1))
		{
			inLock->guard = 0;
		}
		else
		{
			int *tmp = (int*)malloc(sizeof(int));
			*tmp = 1;
			queue_enqueue(inLock->q,tmp);
			inLock->guard = 0;
			while(!__sync_bool_compare_and_swap(&inLock->flag,0,1))
			{
				futex_wait(tmp, 1);
			}
		}
}
void spinlock_release(spinlock_t* inLock)
{
	while(xchg(&inLock->guard,1) == 1);
	if(!queue_empty(inLock->q))
	{
		int *tmp = queue_dequeue(inLock->q);
		futex_post(tmp);
	}
	xchg(&inLock->flag,0);
	inLock->guard = 0;
}

