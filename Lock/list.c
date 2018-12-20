/*************************************************************************
    > File Name: list.c
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Wed 21 Nov 2018 09:05:34 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include "mutex.h"
#include "spinlock.h"
#include "counter.h"
#include "mylock.h"
#include "list.h"
void list_init(list_t *list)
{
	list->root= NULL;
	lockInit(&(list->lck));
}
void list_insert(list_t *list, unsigned int key)
{
	list_value *temp = (list_value*)malloc(sizeof(list_value));
	temp->key= key;
	temp->next = NULL;
	lock(&list->lck);
			temp->next = list->root;
			list->root = temp;
	unlock(&list->lck);
}
void list_delete(list_t *list, unsigned int key)
{
	list_value *temp;
	list_value *pre;
	lock(&list->lck);
		temp = list->root;
		pre  = list->root;
		while(temp!=NULL)
		{
			if(temp->key== key)
			{
				if(pre==temp)
				{
					temp = list->root->next;
					free(list->root);
					list->root = temp;
				}
				else
				{
					pre->next = temp->next;
					free(temp);
				}
				unlock(&list->lck);
				return ;
			}
			pre  = temp;
			temp = temp->next;
		}
	unlock(&list->lck);
}
void *list_lookup(list_t *list, unsigned int key)
{
	list_value *temp ;
	lock(&list->lck);
		temp = list->root;
		while(temp!=NULL)
		{
			if(temp->key == key)
			{
				unlock(&list->lck);
				return temp;
			}
			temp=temp->next;
		}
	unlock(&list->lck);
	return NULL;
}
