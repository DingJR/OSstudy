/*************************************************************************
    > File Name: hash.c
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Tue 20 Nov 2018 08:59:11 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "hash.h"
#include "mutex.h"
#include "list.h"
#include "spinlock.h"
#include "mylock.h"

void hash_init(hash_t *hash, int size)
{
	hash->bucket = (list_t*)malloc(sizeof(list_t)*size);
	hash->size=size;
	int i=0;
	for(;i<size;i++)
	{
		list_init(&(hash->bucket[i]));
	}
}

void hash_insert(hash_t *hash, unsigned int key)
{
	int bucket = key%hash->size;
	list_insert(&(hash->bucket[bucket]),key);
}
void hash_delete(hash_t *hash,unsigned int key)
{
	int bucket = key%hash->size;
	list_delete(&(hash->bucket[bucket]),key);
}
void *hash_lookup(hash_t *hash,unsigned int key)
{
	int bucket = key%hash->size;
	return list_lookup(&(hash->bucket[bucket]),key);
}




