#ifndef _SIGINFO_H_
#define _SIGINFO_H_

#include "aio.h"
#include "list.h"

#define SIGEV_NONE          1
#define SIGEV_SIGNAL        2
#define SIGEV_THREAD        3

union sigval
{
    /* integer value */
    int sival_int;
    /* pointer value */
    void *sival_ptr;
};

typedef void (*sighandler_t)(union sigval);

struct sigevent
{
    /* mask its status */
    int flag;
  
    /* notification type */
    int sigev_notify;
    /* signal number */
    int sigev_signo;

    /* signal handle paramer */
    union sigval sigev_value;

    /* signal handle */
    void (*sigev_notify_function)(union sigval);

    /* signal handle thread atrribute */
    pthread_attr_t *sigev_notify_attributes;
};

struct sigevent_list
{
    list_t              list;
    
    struct sigevent     sigevent;
};

#endif
