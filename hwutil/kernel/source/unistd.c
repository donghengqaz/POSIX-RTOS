/*
 * File         : unistd.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-11       DongHeng        create
 */

#include "unistd.h"
#include "stdlib.h"
#include "string.h"
#include "pthread.h"
#include "sched.h"
#include "stdio.h"
#include "pthread.h"

struct stdobj
{
    /* the stand object node list */
    list_t                      list;
  
    /* the stand object name */
    char                        name[STDOBJ_NAME_MAX];
    
    /* the stand object operations */
    struct stdops               *stdops;
    
    pthread_mutex_t             mutex;
};   

static const char *stdobj_name[] = 
{
    "/dev/",
    "/socket/",
    "/fd/"
};
static const int stdobj_num = sizeof(stdobj_name) / sizeof(stdobj_name[0]);

static list_t stdobj_list[STDOBJ_NUM_MAX] KERNEL_SECTION;

int stdobj_init(void)
{
    int i;
  
    for (i = 0; i < STDOBJ_NUM_MAX; i++)
        list_init(&stdobj_list[i]);
  
    return 0;
}

int stdobj_create (enum stdobj_type obj, const char *name, struct stdops *stdops)
{
    struct stdobj *stdobj;
  
    if (obj >= STDOBJ_NUM_MAX)
        return -EINVAL;
  
    if (!stdops)
        return -EINVAL;
    
    if (!(stdobj = calloc(sizeof(struct stdobj))))
        return -EINVAL;
    
    memcpy(&stdobj->name, name , STDOBJ_NAME_MAX - 1);
    stdobj->stdops = stdops;
    list_init(&stdobj->list);
    pthread_mutex_init(&stdobj->mutex, NULL);
    
    list_insert_tail(&stdobj_list[obj], &stdobj->list);
    
    return 0;
}

int access (const char *name, int flag)
{
    const char *str;
    int type;
    struct stdobj *stdobj;
  
    if (!name)
        return 0;
    
    for (type = 0; type < stdobj_num; type++)
    {
        if (strstr(name, stdobj_name[type]))
            break;
    }
    
    if (type >= stdobj_num)
        return 0;
    
    str = name + strlen(stdobj_name[type]);
    
    LIST_FOR_EACH_ENTRY(stdobj,
                        &stdobj_list[type],
                        struct stdobj,
                        list)
    {
        if (!strcmp(stdobj->name, str))
        {
            stdobj->stdops->flag = flag;
            
            if (stdobj->stdops->open)               
                if (stdobj->stdops->open(stdobj->stdops) < 0)
                    return -1;
          
            return (int)stdobj;
        }
    }
    
    return 0;
}

ssize_t read (int fildes, void *buf, size_t nbyte)
{ 
    struct stdobj *stdobj;
  
    if (fildes <= 0)
        return -EINVAL;
  
    stdobj = (struct stdobj *)fildes;
    
    if (stdobj->stdops->read)
        return stdobj->stdops->read(stdobj->stdops, buf, nbyte, 0);
    else
        return -EINVAL;
}

ssize_t write (int fildes, const void *buf, size_t nbyte)
{ 
    struct stdobj *stdobj;
  
    if (fildes <= 0)
        return -EINVAL;
  
    stdobj = (struct stdobj *)fildes;
    
    if (stdobj->stdops->write)
        return stdobj->stdops->write(stdobj->stdops, buf, nbyte, 0);
    else
        return -EINVAL;
}

loff_t lseek (int fildes, loff_t loff, int from)
{ 
    struct stdobj *stdobj;
  
    if (fildes <= 0)
        return -EINVAL;
  
    stdobj = (struct stdobj *)fildes;
    
    if (stdobj->stdops->lseek)
        return stdobj->stdops->lseek(stdobj->stdops, loff, from);
    else
        return -EINVAL;
}

int ioctl (int fildes, int request, ...)
{
    struct stdobj *stdobj;
    va_list args;
    void *ioctl_arg;
  
    if (fildes <= 0)
        return -EINVAL;
    
    va_start(args, request);
    ioctl_arg = (void *)(*(int *)args);
  
    stdobj = (struct stdobj *)fildes;
    
    if (stdobj->stdops->control)
        return stdobj->stdops->control(stdobj->stdops, request, ioctl_arg);
    else
        return -EINVAL;
}

int lock (int fildes, loff_t offset, size_t length)
{
    struct stdobj *stdobj;
  
    if (fildes <= 0)
        return -EINVAL;

    stdobj = (struct stdobj *)fildes;
    
    return pthread_mutex_lock(&stdobj->mutex);
}

int unlock (int fildes, loff_t offset, size_t length)
{
    struct stdobj *stdobj;
  
    if (fildes <= 0)
        return -EINVAL;
    
    stdobj = (struct stdobj *)fildes;
    
    return pthread_mutex_unlock(&stdobj->mutex);
}

int close (int fildes)
{
    struct stdobj *stdobj;
  
    if (fildes <= 0)
        return -EINVAL;
  
    stdobj = (struct stdobj *)fildes;
    
    if (stdobj->stdops->release)
        return stdobj->stdops->release(stdobj->stdops);
    else
        return 0;
}

/******************************************************************************/

unsigned int msleep(unsigned int milliseconds)
{
    phys_reg_t temp;
    os_pthread_t *current_pthread;
    
    if (!(current_pthread = get_current_thread()))
        return 0;
    
    temp = hw_interrupt_suspend();
    
    current_pthread->sleep_ticks = (milliseconds / RTOS_SYS_TICK_PERIOD) ? milliseconds / RTOS_SYS_TICK_PERIOD : 1;
    sched_set_thread_sleep(current_pthread);
    
    sched_switch_thread();
    
    hw_interrupt_recover(temp);
    
    return milliseconds;
}

unsigned int sleep(unsigned int seconds)
{
    msleep(seconds * 1000);
    
    return seconds;
}

