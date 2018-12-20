/*************************************************************************
    > File Name: list.h
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Tue 20 Nov 2018 07:10:00 PM CST
 ************************************************************************/

#ifndef _LIST_H
#define _LIST_H
#include "mylock.h"
#include "list.h"

typedef struct list_value
{
	unsigned int	   key;
	struct list_value* next;
}list_value;

typedef struct list_t
{
	lock_t lck;
	list_value* root;
}list_t;

void list_init(list_t *list);
void list_insert(list_t *list, unsigned int key);
void list_delete(list_t *list, unsigned int key);
void *list_lookup(list_t *list, unsigned int key);
#endif
