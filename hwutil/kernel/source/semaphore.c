/*
 * File         : semaphore.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-11       DongHeng        create
 */

#include "semaphore.h"
#include "pthread.h"
#include "sched.h"
#include "stdio.h"

#define CONTROLLER_DEBUG_LEVEL  10

#if CONTROLLER_DEBUG_LEVEL
    #define CONTROLLER_DEBUG(level, x) \
    { \
        if (level >= CONTROLLER_DEBUG_LEVEL) \
            printk x; \
    }
    #if CONTROLLER_DEBUG_LEVEL < 10
        volatile int debug_trace[2];
    #endif  
#else
    #define CONTROLLER_DEBUG(level, x)
#endif

/*
 * sem_init - Initialize the semaphore
 *
 * @param sem the semaphore object point
 * @param pshared the atrribute of the semaphore
 * @param value the value of the semaphore lock
 *
 * @return the result
 */
int sem_init (sem_t *sem, int pshared, unsigned int value)
{
    os_u32 temp;
    
    if (value > SEM_VALUE_MAX)
        return -EINVAL;
    
    temp = hw_interrupt_suspend();
    
    sem->value = 0;
    sem->init_value = value;
    
    list_init(&sem->wait_list);
    
    hw_interrupt_recover(temp);
  
    return 0;
}

/*
 * sem_init - suspend the thread if semaphore value is 0, otherwise reduce the value
 *
 * @param sem the semaphore object point
 *
 * @return the result
 */
INLINE int __sem_wait (sem_t* sem)
{
    os_u32 temp = hw_interrupt_suspend();
    int ret = 0;
  
    if (!sem->value)
    {  
        os_pthread_t *thread = get_current_thread();

        sched_set_thread_suspend(thread);
        list_insert_tail(&sem->wait_list, &thread->list);        
        sched_switch_thread();
        
        ret = -EINVAL;
    }
    else
        --sem->value;
    
    hw_interrupt_recover(temp);
          
    return ret;
}

/*
 * sem_init - wait the semaphore to be pluses
 *
 * @param sem the semaphore object point
 *
 * @return the result
 */
int sem_wait (sem_t* sem)
{
    while (__sem_wait(sem))
    {}
          
    return 0;
}

/*
 * sem_init - post the semaphore to be pluses
 *
 * @param sem the semaphore object point
 *
 * @return the result
 */
int sem_post (sem_t* sem)
{
    os_u32 temp;
    os_pthread_t *thread_wait;
  
    if (sem->value >= SEM_VALUE_MAX)
        return -EINVAL;
  
    temp = hw_interrupt_suspend();

    if (sem->value < sem->init_value)
        ++sem->value;
    
    /* wake up one thread in the wait queue */
    LIST_FOR_EACH_HEAD_NEXT(thread_wait,
                            &sem->wait_list,
                            os_pthread_t,
                            list)
    {
        sched_set_thread_ready(thread_wait);
        sched_switch_thread();
        break;
    }
    
    hw_interrupt_recover(temp);
          
    return 0;
}
