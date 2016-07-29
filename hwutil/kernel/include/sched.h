#ifndef _SCHED_H_
#define _SCHED_H_

#include "pthread.h"

void sched_init(void);
void sched_switch_thread(void);
void sched_start(void);

os_u32 sched_suspend(void);
void sched_recover(os_u32 state);

void sched_insert_thread(os_pthread_t *thread);
void sched_set_thread_ready(os_pthread_t *thread);
void sched_set_thread_suspend(os_pthread_t *thread);
void sched_set_thread_sleep(os_pthread_t *thread);
void sched_set_thread_close(os_pthread_t *thread);
void sched_set_thread_priority(os_pthread_t *thread, os_u16 priority);

void sched_reclaim_thread(os_pthread_t *thread);
err_t sched_delete_thread(os_pthread_t *thread);
os_pthread_t* get_current_thread(void);
os_u16 get_current_usage(void);

int __sched_report_signal(os_pthread_t *thread,
                          void *(*start_routine)(void*), 
                          void *RESTRICT arg);

#endif
