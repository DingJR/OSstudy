/*************************************************************************
    > File Name: counter.c
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Tue 20 Nov 2018 09:22:53 PM CST
 ************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include "mutex.h"
#include "spinlock.h"
#include "counter.h"
#include "mylock.h"

void counter_init(counter_t *c, int value)
{
	lockInit(&(c->lck));
	c->value = value;
}
int counter_get_value(counter_t *c)
{
	lock(&c->lck);
	int temp = c->value;
	unlock(&c->lck);
	return temp;
}
void counter_increment(counter_t *c)
{
	lock(&c->lck);
	c->value += 1;
	unlock(&c->lck);
}
void counter_decrement(counter_t *c)
{
	lock(&c->lck);
	c->value -= 1;
	unlock(&c->lck);
}
