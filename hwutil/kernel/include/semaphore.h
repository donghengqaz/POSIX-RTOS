#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "rtos.h"
#include "list.h"

#define SEM_VALUE_MAX               32

struct sem
{
    /* semaphore init count value */
    unsigned int         init_value;
    
    /* semaphore count value */
    unsigned int         value;    
    
    /* thread wait list */
    list_t               wait_list;
};
typedef struct sem sem_t;

int sem_init (sem_t* sem, int pshared, unsigned int value);
int sem_wait (sem_t* sem);
int sem_post (sem_t* sem);

#endif
