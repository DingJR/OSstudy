/*************************************************************************
    > File Name: myque.h
    > Author: DingJR
    > Mail: 1048381723@qq.com 
 ************************************************************************/

#ifndef _MYQUEUE_H
#define _MYQUEUE_H
typedef struct __lnode_t{
	int *value ;
	struct __lnode_t *next;
}lnode_t;

typedef struct __lqueue_t{
	lnode_t *head;
	lnode_t *tail;
}lqueue_t;

void queue_init(lqueue_t *q);
void queue_enqueue(lqueue_t *q,int *val);
int *queue_dequeue(lqueue_t *q);
int queue_empty(lqueue_t *q);


#endif
