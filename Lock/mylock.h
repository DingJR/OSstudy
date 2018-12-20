/*************************************************************************
    > File Name: mylock.h
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Tue 20 Nov 2018 08:55:59 PM CST
 ************************************************************************/

#include "mutex.h"
#include "spinlock.h"
#include "twophase.h"
#include <pthread.h>
#define MYLOCK	      1  //option:0,1,2


#define MY_SPIN_LOCK  0
#define MY_MUTEX_LOCK 1
#define MUTEX_LOCK    2
#define TWO_PHASE_LOCK 3

#if MYLOCK==MY_MUTEX_LOCK
#define lock_t mutex_t
#define lock(mylock) mutex_acquire(mylock)
#define unlock(mylock) mutex_release(mylock)
#define lockInit(mylock) mutex_init(mylock)
#elif MYLOCK==MY_SPIN_LOCK
#define lock_t spinlock_t
#define lock(mylock) spinlock_acquire(mylock)
#define unlock(mylock) spinlock_release(mylock)
#define lockInit(mylock) spinlock_init(mylock)
#elif MYLOCK==MUTEX_LOCK
#define lock_t pthread_mutex_t
#define lock(mylock) pthread_mutex_lock(mylock);
#define unlock(mylock) pthread_mutex_unlock(mylock);
#define lockInit(mylock) pthread_mutex_init(mylock, NULL)
#elif MYLOCK==TWO_PHASE_LOCK
#define lock_t twophase_t
#define lock(mylock) twophase_acquire(mylock);
#define unlock(mylock) twophase_release(mylock);
#define lockInit(mylock) twophase_init(mylock)
#endif

