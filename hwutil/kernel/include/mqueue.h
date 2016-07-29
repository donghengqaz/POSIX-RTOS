#ifndef _MQUEUE_H_
#define _MQUEUE_H_

#include "unistd.h"

/* the atrribute of the message queue */
struct mq_attr
{
#define MQ_MSG_SIZE_MAX             32
#define MQ_MSG_NUM_MAX              16    
  
    long                mq_flags;        //Message queue flags.
    long                mq_maxmsg;       //Maximum number of messages.
    long                mq_msgsize;      //Maximum message size.
    long                mq_curmsgs;      //Number of messages currently queued
};
typedef struct mq_attr mq_attr_t;

mqd_t mq_open (const char *name, int flag, ...);
ssize_t mq_send (mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);

#endif
