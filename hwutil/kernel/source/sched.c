/*
 * File         : shed.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-11-23       DongHeng        create
 */

#include "sched.h"
#include "list.h"
#include "string.h"
#include "debug.h"
#include "stdio.h"

/*@{*/

#define SCHED_DEBUG_LEVEL 0
#define SCHED_DEBUG_ENABLE 0
#define SCHED_DEBUG(level, ...) \
    if (level > SCHED_DEBUG_LEVEL) \
        printk(__VA_ARGS__);

#define SCHED_IS_LOCKED      1
#define SCHED_IS_UNLOCKED    0

/******************************************************************************/

/* CPU usage structure description */
struct usage
{
    /* ticks for a minute */
    os_u16              min_ticks;
    os_u16              cur_ticks;
    
    /* ticks for system is not idle */
    os_u16              run_count;
    
    os_u16              usage;
};

/* scheduler structure description */
struct sched
{
    os_u32              thread_ready_group;
    list_t              thread_ready_table[PTHREAD_READY_GROUP_MAX];
    
    list_t              thread_sleep_list;
    list_t              thread_delete_list;
    
    os_pthread_t        *current_thread;
    list_t              thread_list;
    
    struct usage        usage;
    
    os_u32              locked;
};

/*@}*/

/*@{*/

/* system scheduler structure */
static struct sched sched KERNEL_SECTION;

NO_INIT os_u32 thread_switch_interrupt_flag KERNEL_SECTION;
NO_INIT os_u32 interrupt_from_thread KERNEL_SECTION;
NO_INIT os_u32 interrupt_to_thread KERNEL_SECTION;
/*@}*/

/*@{*/

/**
 * This function will get the highest priority thread
 *
 * @param read_group the read threads record bit group
 *
 * @return thread number which has the highest priority
 */
INLINE os_u16 sched_get_highest_ready_group(os_u32 read_group)
{
	os_u16 offset = 0;
    /* scheduler thread priority mapped table */
    STATIC OS_RO os_u8 sched_priority_remap_table[OS_U8_MAX + 1] =
    {
        0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    };
  
    if (read_group & 0xff000000)
    {
        read_group >>= 24;
        offset = 24;
    }
    else if (read_group & 0x00ff0000)
    {
        read_group >>= 16;
        offset = 16;
    }
    else if (read_group & 0x0000ff00)
    {
        read_group >>= 8;
        offset = 8;
    }
    
    return sched_priority_remap_table[read_group] + offset;
}

/**
 * This function will start the operation system scheduler
 */
void sched_start(void)
{
    extern void hw_context_switch_to(phys_reg_t sp);
  
    os_u32 thread_ready_group_num = sched_get_highest_ready_group(sched.thread_ready_group);
    
    sched.current_thread = LIST_HEAD_ENTRY(&sched.thread_ready_table[thread_ready_group_num],
                                           os_pthread_t,
                                           list);
    
    hw_context_switch_to((phys_reg_t)&sched.current_thread->sp);
}

/**
 * This function will start the operation system scheduler
 */
void sched_switch_thread(void)
{
    phys_reg_t temp;
    os_u32 thread_ready_group_num;
    os_pthread_t *to_thread;
    extern void hw_context_switch(phys_reg_t from_thread, phys_reg_t to_thread);
    
    /* suspend the hardware interrupt for the preparing for context switching */
    temp = hw_interrupt_suspend();
      
    thread_ready_group_num = sched_get_highest_ready_group(sched.thread_ready_group);
    
    to_thread = LIST_HEAD_ENTRY(&sched.thread_ready_table[thread_ready_group_num],
                                os_pthread_t,
                                list);
    
    /* we do context switching if current highest priority thread being changed */
    if (to_thread != sched.current_thread)
    {
        os_pthread_t *from_thread;
        phys_reg_t from_sp, to_sp;
        
        from_thread = sched.current_thread;
        sched.current_thread = to_thread;
        
        SCHED_DEBUG(SCHED_DEBUG_ENABLE, "from thread [0x%08x] to [0x%08x]\r\n",
        									from_thread, to_thread);
        
        if (to_thread->status == PTHREAD_STATE_INT)
        	to_sp = (phys_reg_t)&to_thread->int_sp;
        else
        	to_sp = (phys_reg_t)&to_thread->sp;

        if (from_thread->status == PTHREAD_STATE_INT)
        	from_sp = (phys_reg_t)&from_thread->int_sp;
        else
        	from_sp = (phys_reg_t)&from_thread->sp;
        
        hw_context_switch(from_sp, to_sp);

        SCHED_DEBUG(SCHED_DEBUG_ENABLE, "thread switched.\r\n");
    }
    
    /* recover the system or user interrupt status */
    hw_interrupt_recover(temp);
}

/**
 * This function will wake up the sleeping thread if it is timeout
 */
INLINE void sched_wakeup_sleep_thread(void)
{
    os_pthread_t *pthread, *p;

    LIST_FOR_EACH_ENTRY_SAFE(pthread, 
                             p,
                             &sched.thread_sleep_list,
                             os_pthread_t,
                             list)
    {
        /* active the suspend thread and put it on ready group */
    	pthread->sleep_ticks--;

        /* check the sleeping time */
        if (!pthread->sleep_ticks)
        {
            /* wake up the thread */
            sched_set_thread_ready(pthread);
        }
    }
}

/**
 * This function will wake up the sleeping thread if it is timeout
 */
INLINE void sched_proc_cpu_usage(void)
{
    os_pthread_t *thread = PTHREAD_POINT(get_current_thread()); 

    sched.usage.cur_ticks++;
    
    /* count the ticks when the CPU is running */
    if (PTHREAD_TYPE_IDLE != thread->type)
    	sched.usage.run_count += SCHED_PERIOD / RTOS_SYS_TICK_PERIOD;

    /* when ticks is overflow, compute the CPU usage  */
    if (sched.usage.cur_ticks >= sched.usage.min_ticks)
    {
    	sched.usage.usage = sched.usage.run_count / sched.usage.min_ticks;
      
    	sched.usage.run_count = 0;
    	sched.usage.cur_ticks = 0;
    }
}

/**
 * This function will check if the highest priority of thread scheduler changed
 */
INLINE void sched_proc_current_thread(void)
{
    --sched.current_thread->cur_ticks;
    if (!sched.current_thread->cur_ticks)
    {
        /* remove the thread from the thread-ready group head and insert it to tail */
        sched_set_thread_ready(sched.current_thread);
    }
}

/**
 * This function is the entry function of scheduler timer timeout
 */
void sched_proc(void)
{
    phys_reg_t temp;
    
    /* if the scheduler is lock, return immediately */
    if (SCHED_IS_LOCKED == sched.locked)
        return;
    
    temp = hw_interrupt_suspend();
    
    /* check and handle current thread */
    sched_proc_current_thread();

    /* check and wake up the sleeping thread */
    sched_wakeup_sleep_thread();

    /* switch to the thread which has the highest priority */
    sched_switch_thread();
    
    /* compute the CPU usage */
    sched_proc_cpu_usage();
    
    hw_interrupt_recover(temp);
}

/**
 * This function will initialize the system scheduler
 *
 * @return the result of initialization the scheduler
 */
void sched_init(void)
{
    int i;
    
    /* clear the scheduler structure */
    memset(&sched, 0, sizeof(struct sched));
    
    /* initialize all list of scheduler */
    for( i = 0; i < PTHREAD_READY_GROUP_MAX; i++ )
    {
        list_init( &sched.thread_ready_table[i] );
    }
    list_init(&sched.thread_list);
    list_init(&sched.thread_sleep_list);
    list_init(&sched.thread_delete_list);
    
    /* set CPU usage remark period */
    sched.usage.min_ticks = SCHED_PERIOD / RTOS_SYS_TICK_PERIOD;
}

/**
 * This function will insert the thread into the global thread list
 *
 * @param thread the thread point to be registered
 */
void sched_insert_thread(os_pthread_t *thread)
{ 
    list_insert_tail(&sched.thread_list, &thread->tlist);
    sched_set_thread_ready(thread);
}

/**
 * This function will insert the thread into the thread-ready list and set the group
 *
 * @param thread the thread point to be handled
 */
void sched_set_thread_ready(os_pthread_t *thread)
{ 
    list_remove_node(&thread->list);

    list_insert_tail(&sched.thread_ready_table[thread->cur_prio], &thread->list);
    sched.thread_ready_group |= (1 << thread->cur_prio);
    thread->status = PTHREAD_STATE_READY;
}

/**
 * This function will insert the thread into the thread-ready list and set the
 * status of the thread to be interruptible
 *
 * @param thread the thread point to be handled
 */
void sched_set_thread_int(os_pthread_t *thread)
{ 
    list_remove_node(&thread->list);

    list_insert_tail(&sched.thread_ready_table[thread->cur_prio], &thread->list);
    sched.thread_ready_group |= (1 << thread->cur_prio);
    thread->status = PTHREAD_STATE_INT;
}

/**
 * This function will suspend the thread and move from the thread-ready table 
 * and reset the group scheduler_suspend_thread
 *
 * @param thread the thread point to be handled
 */
void sched_set_thread_suspend(os_pthread_t *thread)
{ 
    list_remove_node(&thread->list); 
    if (list_is_empty(&sched.thread_ready_table[thread->cur_prio]))
    	sched.thread_ready_group &= ~(1 << (thread->cur_prio));
    
    thread->status = PTHREAD_STATE_SUSPEND;
}

/**
 * his function will let the thread sleep
 *
 * @param thread the thread point to be handled
 */
void sched_set_thread_sleep(os_pthread_t *thread)
{
    list_remove_node(&thread->list);
    if (list_is_empty(&sched.thread_ready_table[thread->cur_prio]))
    	sched.thread_ready_group &= ~(1 << (thread->cur_prio));
    
    list_insert_tail(&sched.thread_sleep_list, &thread->list);
    thread->status = PTHREAD_STATE_SLEEP;
}

/**
 * This function will let the thread closed
 *
 * @param thread the thread point to be handled
 */
void sched_set_thread_close(os_pthread_t *thread)
{
    list_remove_node(&thread->list);   
    if (list_is_empty(&sched.thread_ready_table[thread->cur_prio]))
    	sched.thread_ready_group &= ~(1 << (thread->cur_prio));
    
    thread->status = PTHREAD_STATE_CLOSED;
}

/**
 * This function will reclaim the thread closed
 *
 * @param thread the thread point to be handled
 */
void sched_reclaim_thread(os_pthread_t *thread)
{
    sched_set_thread_close(thread);
    
    list_insert_tail(&sched.thread_delete_list, &thread->list);
}

/**
 * This function will return the current thread point
 *
 * return the current thread point
 */
os_pthread_t* get_current_thread(void)
{
    return sched.current_thread;
}

/**
  * This function will delete the thread from the scheduler table
  *
  * @param thread the thread point to be handled
  *
  * return the result
  */
err_t sched_delete_thread(os_pthread_t *thread)
{
    phys_reg_t temp;  
  
    /* suspend the hardware interrupt for atomic operation */
    temp = hw_interrupt_suspend();

    /* remove the thread from the global thread group list */
    list_remove_node(&thread->tlist);

    /* remove the thread from the ready thread group list */
    sched_reclaim_thread(thread);

    hw_interrupt_recover(temp);

    sched_switch_thread();
  
    return 0;
}


/**
 * sched_yield - This function will let the current thread jump into the tail   
 *               of the thread-ready list
 */
void sched_yield(void)
{
    phys_reg_t temp;
  
    /* suspend the hardware interrupt for atomic operation */
    temp = hw_interrupt_suspend();
    
    /* remove the current thread from the ready thread
       and put it to the tail of the ready group */
    sched_set_thread_ready(get_current_thread());
       
    hw_interrupt_recover(temp);
    
    sched_switch_thread();
}

/**
  * the function will suspend the scheduler
  *
  * @return the previous scheduler lock status
  */
os_u32 sched_suspend(void)
{
    phys_reg_t temp;
    os_u32 lcoked;
  
    temp = hw_interrupt_suspend();
  
    lcoked = sched.locked;
    sched.locked = SCHED_IS_LOCKED;
  
    hw_interrupt_recover(temp);
  
    return lcoked;
}

/**
  * the function will recover the scheduler status
  *
  * @param state the status of the scheduler lock
  */
void sched_recover(os_u32 state)
{
    phys_reg_t temp;
  
    temp = hw_interrupt_suspend();
  
    sched.locked = state;
  
    hw_interrupt_recover(temp);
}

/**
 * This function will return the current CPU usage
 *
 * @return the current CPU usage
 */
os_u16 get_current_usage(void)
{
    return sched.usage.usage;
}

/**
  * the function will report the current thread
  */
void sched_status_report(void)
{
    os_pthread_t *pthread = get_current_thread();
    
    if (!pthread->name[0])
        printk("current thread id is 0x%08x.\r\n", pthread);
    else
        printk("current thread name is %s.\r\n", pthread->name);
}

/**
  * the function will report the current thread
  *
  * @param thread the thread point to be handled
  * @param start_routine the thread entry function point
  * @param arg the param of the thread
  */
int __sched_report_signal(os_pthread_t *thread,
                          void *(*start_routine)(void*), 
                          void *RESTRICT arg)
{
    phys_reg_t temp;
    
    if (thread->status == PTHREAD_STATE_INT)
        return -EINVAL;
  
    temp = hw_interrupt_suspend();
    
    __pthread_int_init(thread, start_routine, arg);
    
    sched_set_thread_int(thread);
    
    sched_switch_thread();
  
    hw_interrupt_recover(temp);
    
    return 0;
}

/*@}*/
