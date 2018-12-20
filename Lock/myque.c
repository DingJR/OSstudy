/*************************************************************************
    > File Name: twophase.c
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Thu 22 Nov 2018 10:42:57 PM CST
 ************************************************************************/

#include <stdio.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "myque.h"
uint xchg(volatile unsigned short *addr, unsigned int newval)
{
    uint result;
    asm volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
    return result;
}


void queue_init(lqueue_t *q)
{
	lnode_t *tmp = (lnode_t*)malloc(sizeof(lnode_t));
	tmp->value = 0;
	tmp->next = NULL;
	q->head = tmp;
	q->tail = tmp;
}
void queue_enqueue(lqueue_t *q,int * val)
{
	lnode_t *tmp = (lnode_t*)malloc(sizeof(lnode_t));
	if(q==NULL || tmp ==NULL)
	{
		exit(1);
	}
	tmp->value = val;
	tmp->next = NULL;
	lnode_t *r;
	do
	{
		r = q->tail;
	}while(!__sync_bool_compare_and_swap(&(r->next),NULL, tmp));
	__sync_bool_compare_and_swap(&(q->tail),r,tmp);
}
int* queue_dequeue(lqueue_t *q)
{
	lnode_t* p;
	do
	{
		p = q->head;
	}while(!__sync_bool_compare_and_swap(&(q->head),p,p->next));
	*(p->next->value) = 0;
	return p->next->value;
}
int queue_empty(lqueue_t *q)
{
	if(q==NULL || q->head==q->tail )
		return 1;
	return 0;
}













