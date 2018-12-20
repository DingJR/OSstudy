/*************************************************************************
    > File Name: twophase.c
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Thu 22 Nov 2018 10:42:57 PM CST
 ************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "futex_wrapper.h"
#include "spinlock.h"
#include "mutex.h"
#include "twophase.h"
uint xchg(volatile unsigned short *addr, unsigned int newval)
{
    uint result;
    asm volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
    return result;
}

void twophase_init(twophase_t *mylock)
{
	*mylock = 0;
}
void twophase_acquire(twophase_t *mylock)
{
	int state = __sync_val_compare_and_swap(mylock,UNLOCKED,LOCKED);
	//if not locked
	while(state)
	{
		//if it is now congested or locked
		if(state == COMPETE || __sync_val_compare_and_swap(mylock, LOCKED, COMPETE))
			futex_wait(mylock,COMPETE);
		for(int j=0;j<10;j++)
		{
			state = __sync_val_compare_and_swap(mylock, UNLOCKED, COMPETE);
			if(!state)return;
		}
	}
}
void twophase_release(twophase_t *mylock)
{
	if( __sync_lock_test_and_set(mylock,0) == COMPETE)
		futex_post(mylock);
}

