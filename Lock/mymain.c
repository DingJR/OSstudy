/*************************************************************************
    > File Name: mymain.c
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Tue 20 Nov 2018 09:31:25 PM CST
 ************************************************************************/


#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/sysinfo.h>
#include<unistd.h>

#define __USE_GNU
#include<sched.h>
#include<ctype.h>
#include<string.h>

#include <malloc.h>
#include <sys/time.h>
#include "counter.h"
#include "hash.h"
#include "list.h"
#include "mylock.h"
//#define __USE_CPU_BOND

int thread_Num = 8; //option: 1,4,8,12,16,20
#define LIBCHOICE 2 //option: 0,1,2


#define COUNTERH  0
#define LISTH     1
#define HASHH	  2
#define COMPUTEINTENSIVE 3

#if   LIBCHOICE==LISTH
#define dothing listSomeThing
#define TYPELIB list_t
#define Init(a) list_init(a)
#elif LIBCHOICE==HASHH
#define dothing hashSomeThing
#define TYPELIB hash_t
#define Init(a) hash_init(a,100)
#elif LIBCHOICE==COUNTERH
#define dothing counterSomeThing
#define TYPELIB counter_t
#define Init(a) counter_init(a,1)
#else
#define dothing intensiveCompute
#define TYPELIB counter_t
#define Init(a) counter_init(a,1)
#endif
#define COMPLEX 1e6
typedef struct myarg
{
	int thread_id;
	TYPELIB* strc;
}myarg;
void *counterSomeThing(void* in)
{
#if LIBCHOICE==COUNTERH
	counter_t *ct = (counter_t*)((myarg*)in)->strc;
	int id = ((myarg*)in)->thread_id;

#ifdef __USE_CPU_BOND
    cpu_set_t mask;  
    CPU_ZERO(&mask);
    CPU_SET(id,&mask);
    sched_setaffinity(0, sizeof(mask), &mask);
    cpu_set_t get;  
    CPU_ZERO(&get);
    sched_getaffinity(0, sizeof(get), &get);
	//if(CPU_ISSET(id,&get))printf("%d:1\n",id);
#endif

	printf("Thread %d,%d\n",id,counter_get_value(ct));
	for(int i=0;i<COMPLEX;i++)
		counter_increment(ct);
	printf("Thread %d,%d\n",id,counter_get_value(ct));
	for(int i=0;i<COMPLEX;i++)
		counter_decrement(ct);
	printf("Thread %d,%d\n",id,counter_get_value(ct));
#endif
	return NULL;
}
void *listSomeThing(void* in)
{
#if LIBCHOICE==LISTH
	int id = ((myarg*)in)->thread_id;

#ifdef __USE_CPU_BOND
    cpu_set_t mask;  
    CPU_ZERO(&mask);
    CPU_SET(id,&mask);
    sched_setaffinity(0, sizeof(mask), &mask);
    cpu_set_t get;  
    CPU_ZERO(&get);
    sched_getaffinity(0, sizeof(get), &get);
	if(CPU_ISSET(id,&get))printf("%d:1\n",id);
#endif

	list_t *ml = (list_t*)(((myarg*)in)->strc);
	int i=0;
	for(i=0;i<COMPLEX;i++)
	{
		list_insert(ml,id);
		list_delete(ml,id);
	}
	//for(i=0;i<COMPLEX;i++)
		//list_lookup(ml,id);

	printf("Thread %d,%p\n",id,list_lookup(ml,id));
#endif
	return NULL;
}
lock_t tempLock={0};
void *intensiveCompute(void *in)
{
#if LIBCHOICE==COMPUTEINTENSIVE
	for(int j=0;j<1000;j++)
	{
		lock(&tempLock);
		double count=24.1;
		for(int i=0;i<1000000;i++)count*=count;
		unlock(&tempLock);
	}
#endif
	return NULL;
}
void *hashSomeThing(void* in)
{
#if LIBCHOICE==HASHH
	hash_t *ml = (hash_t*)(((myarg*)in)->strc);
	int id = ((myarg*)in)->thread_id;
	printf("Thread %d, here's hash\n",id);
	for(int i=0;i<10000000;i++)
	{
		hash_insert(ml,id);
		hash_delete(ml,id);
	}
	printf("Thread %d,%p\n",id,hash_lookup(ml,id));
#endif
	return NULL;
}
#include <ctype.h>
int main(int argc,char**argv)
{
	thread_Num = atoi(argv[1]);
	pthread_t tid[thread_Num+1];
	TYPELIB *ml = (TYPELIB*)malloc(sizeof(TYPELIB));
	Init(ml);
	struct timeval tv1,tv2;
	gettimeofday(&tv1, NULL);
	int i=0;
	myarg* myargs[thread_Num + 2];
	for(i=0;i<thread_Num;i++)
	{
		myarg *temp=(myarg*)malloc(sizeof(myarg));
		temp->thread_id = i;
		temp->strc      = ml;
		myargs[i]=temp;
	}
	for(i=0;i<thread_Num;i++)
	{
		pthread_create(&(tid[i]), NULL, &dothing, (myargs[i]));
	}
	for(i=0;i<thread_Num;i++)
	{
		pthread_join(tid[i],NULL);
		free(myargs[i]);
	}
	gettimeofday(&tv2, NULL);
	long long timeInterval = (long long)(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	printf("Time Consuming %lf\n",(double)timeInterval/1000);
	FILE* output = fopen("mutex_output","a+");
	fprintf(output,"%lf\n",(double)timeInterval/1000);
	return 0;
}
