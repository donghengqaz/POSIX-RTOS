/*
 * File         : time.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-11       DongHeng        create
 */

#include "time.h"
#include "stdlib.h"
#include "string.h"
#include "list.h"
#include "semaphore.h"
#include "debug.h"
#include "pthread.h"
#include "sched.h"

#define TIMER_THREAD_STACK_SIZE         512U
#define TIME_DEBUG_LEVEL  10

#if TIME_DEBUG_LEVEL  && TIME_DEBUG_LEVEL < 10
    #define TIME_DEBUG(level, x) \
    { \
        if (level >= TIME_DEBUG_LEVEL) \
            printk x; \
    }
    static volatile int time_trace;
#else
    #define TIME_DEBUG(level, x)
#endif

#define ITIMERSPECT_COPY(form, to) \
            (form)->it_interval.tv_nsec = (to)->it_interval.tv_nsec; \
            (form)->it_interval.tv_sec  = (to)->it_interval.tv_sec; \
            (form)->it_value.tv_nsec    = (to)->it_value.tv_nsec; \
            (form)->it_value.tv_sec     = (to)->it_value.tv_sec; \

struct __timer
{
    /* timer's node list */
    list_t              list;
  
    /* timer's source clock */
    clockid_t           clockid;
  
    /* timer's time plan */
    struct itimerspect  itimerspect;
  
    /* timer's signal trigger */
    struct sigevent     igevent;
};

static list_t timer_list;
static sem_t timer_sem;
static os_u64 local_time;

INLINE timer_t __timer_create(clockid_t clockid, 
                              struct sigevent *RESTRICT evp)
{
    struct __timer *timer;
    phys_reg_t temp;
    
    if (!(timer = malloc(sizeof(struct __timer))))
        return 0;
  
    memcpy(&timer->igevent, evp, sizeof(struct sigevent));
    timer->clockid = clockid;
    
    list_init(&timer->list);
    
    /* suspend the hardware interrupt for the preparing for context switching */
    temp = hw_interrupt_suspend();
    
    list_insert_tail(&timer_list, &timer->list);
    
    hw_interrupt_recover(temp);
    
    return (timer_t)timer;
}

int timer_create (clockid_t clockid, struct sigevent *RESTRICT evp, timer_t *RESTRICT timerid)
{
    *timerid = 0;
  
    if (CLOCK_REALTIME != clockid)
        return -EINVAL;
    
    if (!evp)
        return -EINVAL;
    
    switch (evp->sigev_notify)
    {
        case SIGEV_THREAD:
                    *timerid = __timer_create(clockid, evp);
                    break;
        default:
                    break;
    }
  
    return 0;
}

int timer_settime (timer_t timerid, 
                   int flags, 
                   const struct itimerspect *value, 
                   struct itimerspect *ovalue)
{
    struct __timer *timer = (struct __timer *)timerid;
    phys_reg_t temp;
    
    if (!timerid || !value)
        return -EINVAL;
  
    /* suspend the hardware interrupt for the preparing for context switching */
    temp = hw_interrupt_suspend();
    
    if (ovalue)
        ITIMERSPECT_COPY(ovalue, &timer->itimerspect);
    
    ITIMERSPECT_COPY(&timer->itimerspect, value);
    
    hw_interrupt_recover(temp);
  
    return 0;
}

int clock_settime (clockid_t, const struct timespec *);

int timer_gettime (timer_t timerid, struct itimerspect *value)
{
    return 0; 
}

int timer_delete (timer_t timerid)
{
    return 0;
}

/******************************************************************************/

static void* timer_thread_entry(void *p)
{
    struct __timer *timer;
  
    while (1)
    {
        sem_wait(&timer_sem);
        
        LIST_FOR_EACH_ENTRY(timer,
                            &timer_list,
                            struct __timer,
                            list)
        {
            if (timer->itimerspect.it_value.tv_nsec > 0)
            {
                timer->itimerspect.it_value.tv_nsec -= RTOS_SYS_TICK_PERIOD;
                if (!timer->itimerspect.it_value.tv_nsec)
                {
                    timer->igevent.sigev_notify_function(timer->igevent.sigev_value);
                
                    if (timer->itimerspect.it_interval.tv_nsec)
                        timer->itimerspect.it_value.tv_nsec = timer->itimerspect.it_interval.tv_nsec;
                }
            }
        }
    }
}

int timer_init(void)
{
    int err;
    int tid;
    pthread_attr_t attr;
    sched_param_t timer_sched_param =
      SCHED_PARAM_INIT(PTHREAD_TYPE_KERNEL,
                       PTHREAD_TICKS_MIN,
                       PTHREAD_PRIORITY_MAX);
    
    pthread_attr_setschedparam(&attr, &timer_sched_param);
    pthread_attr_setstacksize(&attr, TIMER_THREAD_STACK_SIZE);
    
    err = pthread_create(&tid,
                         &attr,
                         timer_thread_entry,
                         NULL);
    ASSERT_KERNEL(!err);  
    pthread_setname_np(tid, "timer");
    
    list_init(&timer_list);
  
    sem_init(&timer_sem, 0, 1);
    
    return 0;
}

void os_timetick(void)
{
    extern void sched_proc(void);
        
    sched_proc();
  
    sem_post(&timer_sem);
    
    local_time += RTOS_SYS_TICK_PERIOD;
}

struct tm *localtime_r(const time_t *time, struct tm *RESTRICT result)
{
    os_u64 temp;
  
    result->tm_ms = local_time % 1000;
    
    temp = local_time / 1000;
    
    result->tm_sec = temp % 60;
    temp = temp / 60;
    
    result->tm_min = temp % 60;
    temp = temp / 60;
    
    result->tm_hour = temp % 24;
    temp = temp / 24;
    
    result->tm_wday = temp % 365;
  
    return result;
}
