/*************************************************************************
    > File Name: counter.h
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Tue 20 Nov 2018 07:05:26 PM CST
 ************************************************************************/

#ifndef _counter_H
#define _counter_H
#include "mylock.h"
typedef struct counter_t
{
	lock_t lck;
	int value;
}counter_t;

void counter_init(counter_t *c, int value);
int  counter_get_value(counter_t *c);
void counter_increment(counter_t *c);
void counter_decrement(counter_t *c);

#endif
