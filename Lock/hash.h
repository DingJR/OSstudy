/*************************************************************************
    > File Name: hash.h
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Tue 20 Nov 2018 08:53:58 PM CST
 ************************************************************************/
#ifndef _HASH_H
#define _HASH_H
#include "mylock.h"
#include "spinlock.h"
#include "mylock.h"
#include <pthread.h>
#include "list.h"

typedef struct hash_t
{
	int size;
	list_t *bucket;
}hash_t;
void hash_init(hash_t *hash, int size);
void hash_insert(hash_t *hash, unsigned int key);
void hash_delete(hash_t *hash, unsigned int key);
void *hash_lookup(hash_t *hash, unsigned int key);

#endif
