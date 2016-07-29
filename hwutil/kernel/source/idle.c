/*
 * File         : idle.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-11-25       DongHeng        create
 */

#include "pthread.h"
#include "debug.h"
#include "errno.h"

#ifndef IDLE_STACK_SIZE
    #define IDLE_STACK_SIZE 256
#endif

/*@{*/   

/**
 * It has no necessary to clear the block memory, and you may put it
 * at the place of kernel.
 */
NO_INIT static char idle_stack[IDLE_STACK_SIZE] KERNEL_SECTION;

/*@}*/

/*@{*/

/**
 * The function is the entry of system idle thread, it has the lowest
 * priority in the POSIX-RTOS, when all thread is suspend it will run.
 *
 * @param p user private point
 */
void* idle_thread_entry(void *p)
{ 
    while(1)
    { ; }
}

/*
 * the function will initialize and start the thread of idle function
 *
 * @return the result
 */
err_t idle_thread_init(void)
{
    int err;
    int tid;
    pthread_attr_t attr;
    sched_param_t idle_sched_param =
      SCHED_PARAM_INIT(PTHREAD_TYPE_IDLE,
                       PTHREAD_TICKS_MAX,
                       PTHREAD_PRIORITY_MIN);
    
    pthread_attr_setschedparam(&attr, &idle_sched_param);
    pthread_attr_setstack(&attr, &idle_stack, IDLE_STACK_SIZE);
    
    err = pthread_create(&tid,
                         &attr,
                         idle_thread_entry,
                         NULL);
    ASSERT_KERNEL(!err);
    pthread_setname_np(tid, "idle");

    return 0;
}

/*@}*/
