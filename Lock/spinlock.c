/*************************************************************************
    > File Name: spinlock.c
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Wed 21 Nov 2018 06:29:48 PM CST
 ************************************************************************/

#include <stdio.h>
#include "spinlock.h"
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define cmpxchg(P,O,N) __sync_val_compare_and_swap((P),(O),(N))
#define ATMXADD(P,V) __sync_fetch_and_add((P),(V))
#define BARRIER    asm("":::"memory")
#define SYSPAUSE   asm("pause":::"memory")
uint xchg(volatile unsigned short *addr, unsigned int newval)
{
    uint result;
    asm volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
    return result;
}
#ifdef TICKET
void spinlock_init(spinlock_t* inLock)
{
	inLock->ticket= 0;
	inLock->user= 0;
}
void spinlock_acquire(spinlock_t* inLock)
{
	unsigned iam = ATMXADD(&(inLock->user),1);
		//printf("%d %d %d\n",iam,inLock->ticket,inLock->user);
	while(inLock->ticket!=iam)
	{
		//printf("%u %u\n",xchg(&(inLock->ticket),inLock->ticket),iam);
		//sched_yield();
	}
}
void spinlock_release(spinlock_t* inLock)
{
	inLock->ticket++;
}

#else
void spinlock_init(spinlock_t* inLock)
{
	*inLock= 0;
}
void spinlock_acquire(spinlock_t* inLock)
{
	//while(xchg(inLock,1));//SYSPAUSE;
	while(xchg(inLock,1))sched_yield();
}
void spinlock_release(spinlock_t* inLock)
{
	*inLock= 0;
}
#endif
/*
void spinlock_init(spinlock_t* inLock)
{
	inLock->spin= 0;
	inLock->root = NULL;
}
void spinlock_acquire(spinlock inLock)
{
	static spinlock my = (spinlock)malloc(sizeof(spinlock_t));
	spinlock tail;
	my->next = NULL;
	my->spin = 0;
	tail = xchg(inLock,my);
	if(inLock->flag==0)
	{
		inLock->flag = 1;
		inLock->guard = 0;
	}
	else
	{
		queue_t* temp = (queue_t*)malloc(sizeof(queue_t));
		temp->next=inLock->root;
		temp->id = getpid();
		while(cmpxchg(temp->id,inLock->root->id,0))sched_yield();
		inLock->root = temp;
		inLock->guard = 0;
	}
}
void spinlock_release(spinlock_t* inLock)
{
	while(xchg(&(inLock->guard),1));
	if(xchg((unsigned*)inLock->root,0)!=0)
	{
		inLock->flag = 0;
		queue_t* temp = inLock->root->next;
		free(inLock->root);
		inLock->root = temp;
	}
	inLock->guard = 0;
}
*/

