/*************************************************************************
    > File Name: mutex.c
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Thu 22 Nov 2018 03:50:44 PM CST
 ************************************************************************/

#include <stdio.h>
#include <sys/syscall.h>
#include "mutex.h"
#include <linux/futex.h>
#include <time.h>
#include "futex_wrapper.h"
void mutex_init (mutex_t* mutex)
{
	*mutex = 0;
}
void mutex_acquire (mutex_t* mutex)
{
#ifdef THREE_PHASE
	//check if mutex is zero, if it is, unlocked to locked
	//state==2 means that it's congesting now state == 1 means that it will be locked
	//in next visit
	int state = __sync_val_compare_and_swap(mutex,UNLOCKED,LOCKED);
	//if not locked 
	while(state)
	{
		//if it is now congested or locked
		if(state == COMPETE || __sync_val_compare_and_swap(mutex, LOCKED, COMPETE))
			futex_wait(mutex,COMPETE);
		state = __sync_lock_test_and_set(mutex, LOCKED);
		//state = __sync_val_compare_and_swap(mutex, UNLOCKED, COMPETE);
	};
#elif defined SYSTEMIMPLEMENT
	if (atomic_bit_test_set (mutex,31) == 0) return;
	atomic_increment(mutex);
	int v;
	while(atomic_bit_test_set (mutex,31) != 0)
	{
		v = *mutex;
		if( v >= 0 ) continue;
		futex_wait (mutex,v);
	}
	atomic_decrement (mutex);
	return;
#endif
}
inline int xchg(volatile int*addr, int newval)
{
	int result;
    asm volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");

	return result;
}
#define COFACTOR 0x7fffffff
void mutex_release(mutex_t *mutex)
{
#ifdef THREE_PHASE
	//if mutex-1 not equal 0
	//some threads may have been blocked
	//post them
	//if(__sync_sub_and_fetch(mutex,1))
	if( __sync_lock_test_and_set(mutex,0) == COMPETE)
		futex_post(mutex);
	//else return
#elif defined SYSTEMIMPLEMENT
	if( __sync_fetch_and_and(mutex,COFACTOR) )
	{
		futex_post(mutex);
	}

#endif
}

