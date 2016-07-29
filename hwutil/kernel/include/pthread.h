#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#include "list.h"
#include "signal.h"

/******************************************************************************/


/******************************************************************************/


/******************************************************************************/

/*
 *
 */
struct sched_param
{ 
#define PTHREAD_STATE_INIT        0
#define PTHREAD_STATE_READY       1
#define PTHREAD_STATE_SUSPEND     2
#define PTHREAD_STATE_SLEEP       3
#define PTHREAD_STATE_CLOSED      4
#define PTHREAD_STATE_INT         5
  
#define PTHREAD_TYPE_USER         0      
#define PTHREAD_TYPE_IDLE         1
#define PTHREAD_TYPE_KERNEL       2 

#define PTHREAD_TICKS_MIN         1
#define PTHREAD_TICKS_MAX         255  
  
#define PTHREAD_PRIORITY_MIN      0
#define PTHREAD_PRIORITY_MAX      31

#define PTHREAD_READY_GROUP_MAX   PTHREAD_PRIORITY_MAX + 1
  
    pthread_type_t          type;
    
    /* thread tick slice */
    os_u8                   init_ticks;

    /* thread priority */
    os_u8                   init_prio;
};
typedef struct sched_param sched_param_t;

struct os_pthread
{
    pthread_type_t          type;
    pthread_flag_t          status;
    
    char                    name[STDOBJ_NAME_MAX];
    
    list_t                  list;
    list_t                  tlist;
    
    /* thread phy addr stack point */
    char                    *sp;
    char                    *int_sp;
    
    /* thread tick slice */
    os_u8                   init_ticks;
    os_u8                   cur_ticks;
    
    /* thread sleep ticks */
    os_u16                  sleep_ticks;

    /* thread priority */
    os_u8                   init_prio;
    os_u8                   cur_prio;
    os_u32                  prio_mask;  
    
    /* thread function and user data */
    void*                   (*start_routine)(void *);
    void                    *arg;
    void                    *ret;
    
    /* thread stack info */
    char                    *stk_addr;
    size_t                  stk_size;
    
    /* used for soft push signal event */
    list_t                  sigevent_list;
    
    /* used signal event table */
    list_t                  sig_list;
};
typedef struct os_pthread   os_pthread_t;

#define PTHREAD_POINT(x) \
            ((os_pthread_t *)x)

#define PTHREAD_HD(x) \
            ((pthread_t)x)
              
#define SCHED_PARAM_INIT(type, ticks, prio) \
            {type, ticks, prio}
              
int pthread_create (pthread_t *RESTRICT pthread, 
                    const pthread_attr_t *RESTRICT pthread_attr,
                    void *(*start_routine)(void*), 
                    void *RESTRICT arg);
              
int pthread_attr_setstack (pthread_attr_t *RESTRICT attr, 
                           void *stack_addr, 
                           size_t stack_size);

int pthread_attr_setstacksize(pthread_attr_t *RESTRICT attr, 
                              size_t stack_size);

int pthread_attr_setschedparam(pthread_attr_t *RESTRICT attr,
                               const struct sched_param *restrict param);

int pthread_setname_np(pthread_t thread, const char *name);

int __pthread_int_init (os_pthread_t *pthread, 
                        void *(*start_routine)(void*), 
                        void *RESTRICT arg);
/******************************************************************************/

#ifndef MUTEX_RECURSIVE_MAX
    #define MUTEX_RECURSIVE_MAX  128
#endif

/*
 *  the definition of mutex and mutex data type  
 */
typedef unsigned short       mutex_count_t;

/* the mutex structure description */
struct pthread_mutex
{   
    /* the thread which hold the mutex */
    os_pthread_t             *own_thread;
    
    /* the list which is insert the suspend thread */
    list_t                   wait_list;
};
typedef struct pthread_mutex pthread_mutex_t;

typedef char                 pthread_mutexattr_t;

int pthread_mutex_init (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock (pthread_mutex_t *mutex);
int pthread_mutex_unlock (pthread_mutex_t * mutex);

#endif
