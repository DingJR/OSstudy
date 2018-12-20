/*************************************************************************
    > File Name: mutex.h
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Tue 20 Nov 2018 09:30:45 PM CST
 ************************************************************************/

#ifndef _MUTEX_H
#define _MUTEX_H
#define  UNLOCKED 0
#define  LOCKED   1
#define  COMPETE  2
//#define  THREE_PHASE
#define  SYSTEMIMPLEMENT

typedef int mutex_t;
void mutex_init (mutex_t* mutex);
void mutex_acquire (mutex_t*mutex) ;
void mutex_release (mutex_t*mutex) ;

#endif
