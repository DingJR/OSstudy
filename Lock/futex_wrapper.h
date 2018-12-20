/*************************************************************************
    > File Name: futex_wrapper.h
    > Author: DingJR
    > Mail: 1048381723@qq.com 
    > Created Time: Thu 22 Nov 2018 04:11:41 PM CST
 ************************************************************************/
#ifndef _FUTEX_WRAPPER_H
#define _FUTEX_WRAPPER_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <sys/time.h>
#define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  __sync_val_compare_and_swap (mem, oldval, newval)

#define atomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
     ({__typeof (oldval) __old = (oldval);				      \
     atomic_compare_and_exchange_val_acq (mem, newval, __old) != __old;	})

#define atomic_exchange_acq(mem, newvalue) \
  ({ __typeof (*(mem)) __oldval;					      \
     __typeof (mem) __memp = (mem);					      \
     __typeof (*(mem)) __value = (newvalue);				      \
     do									      \
       __oldval = (*__memp);						      \
     while (__builtin_expect (atomic_compare_and_exchange_bool_acq (__memp,   \
								    __value,  \
								    __oldval),\
			      0));					      \
     __oldval; })

#define atomic_bit_test_set(mem, bit) \
  ({ __typeof (*(mem)) __oldval;					      \
     __typeof (mem) __memp = (mem);					      \
     __typeof (*(mem)) __mask = ((__typeof (*(mem))) 1 << (bit));	      \
     do									      \
       __oldval = (*__memp);						      \
     while (__builtin_expect (atomic_compare_and_exchange_bool_acq (__memp,   \
								    __oldval  \
								    | __mask, \
								    __oldval),\
			      0));					      \
									      \
     __oldval & __mask; })

#define atomic_increment(P) __sync_add_and_fetch((P),1)
#define atomic_decrement(P) __sync_sub_and_fetch((P),1)
int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
    return syscall(SYS_futex, uaddr, futex_op, val,timeout, uaddr, val3);
}

void futex_wait(int * mtx,int v)
{
			//if (__sync_bool_compare_and_swap(mtx, 1, 0))
                   //break;      /* Yes */
    //futex(mtx, FUTEX_WAIT,FUTEX_WAIT_PRIVATE, NULL, NULL, 0);
	//printf("%d %d",FUTEX_WAIT_PRIVATE,FUTEX_WAKE_PRIVATE);
#ifdef SYSTEMIMPLEMENT
    futex(mtx,FUTEX_WAIT,v, NULL, NULL, 0);
#else
    futex(mtx,FUTEX_WAIT,2, NULL, NULL, 0);
#endif
}

void futex_post(int *mtx)
{
    //if (__sync_bool_compare_and_swap(futexp, 0, 1)) 
    futex(mtx, FUTEX_WAKE, 1, NULL, NULL, 0);
}
#endif
