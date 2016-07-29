/*
 * File         : mqueue.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-11       DongHeng        create
 */

#include "mqueue.h"
#include "pthread.h"
#include "sched.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "semaphore.h"

/* class of the message queue */
struct mq
{
    char                *mq_pbuf;
    size_t              mq_pbuf_size;
    
    char                *send_ptr;
    char                *recv_ptr;

    struct mq_attr      mq_attr;
    
    sem_t               sem;

    pthread_mutex_t     r_mutex;
    pthread_mutex_t     w_mutex;
};
typedef struct mq mq_t;

/* class of the message of the message queue */
struct mq_msg
{
    size_t              msg_size;
    int                 used;
};
typedef struct mq_msg mq_msg_t;

/******************************************************************************/

/*
 * mq_open - open a message queue
 *
 * @param name    the name of the message queue
 * @param flag    the flag of the attribute of the message queue
 * @param mode    the mode of the attribute of the message queue
 * @param mq_attr the attribute of the message queue 
 *
 * @return the result
 */
mqd_t mq_open (const char *name, int flag, ...)
{
    va_list args;
    mode_t *mode;
    struct mq_attr *mq_attr;
    mq_t *mq;
    char *buffer;
  
    /* check name and flag */
    if (!name || !(flag & O_CREAT))
    	goto check_failed;
    
    /* check mode */
    va_start(args, flag);   
    mode = (mode_t *)args; 
    if (*mode != flag)
        goto check_failed;
  
    /* check attribute */
    va_arg(args, struct mq_attr *);
    mq_attr = (struct mq_attr *)(*(int *)args);
    if ((mq_attr->mq_maxmsg * mq_attr->mq_msgsize) > (MQ_MSG_SIZE_MAX * MQ_MSG_NUM_MAX))
        goto check_failed;
    
    /* get message buffer */
    if (!(buffer = calloc((sizeof(mq_msg_t) + mq_attr->mq_msgsize) * mq_attr->mq_maxmsg)))
        goto check_failed;
    
    /* get message queue object buffer */
    if (!(mq = calloc(sizeof(mq_t))))
    	goto alloc_buf_failed;
     
    /* initialize the message queue */
    mq->mq_pbuf = buffer;
    memcpy(&mq->mq_attr, mq_attr, sizeof(struct mq_attr));
    mq->send_ptr = mq->recv_ptr = mq->mq_pbuf;
    mq->mq_attr.mq_flags = flag;
    mq->mq_attr.mq_curmsgs = 0;
    mq->mq_pbuf_size = (sizeof(mq_msg_t) + mq_attr->mq_msgsize) * mq_attr->mq_maxmsg;
    
    if (!(mq->mq_attr.mq_flags & O_NONBLOCK))
        if (sem_init(&mq->sem, 0, mq->mq_attr.mq_maxmsg))
            goto sem_init_failed;

    pthread_mutex_init(&mq->r_mutex, NULL);
    pthread_mutex_init(&mq->w_mutex, NULL);

    /* return the message queue address */
    return (mqd_t)mq;

sem_init_failed:
	free(mq);
alloc_buf_failed:
	free(buffer);
check_failed:

	return EINVAL;
}

/*
 * mq_send - send a message to the message queue
 *
 * @param mqdes     the handle oft he message queue
 * @param msg_ptr   send message point
 * @param msg_len   send message length
 * @param msg_prio  send message priority
 *
 * @return the result
 */
ssize_t mq_send (mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio)
{
    mq_t *mq = (mq_t *)mqdes;
    mq_msg_t *mq_msg;
    char *send_ptr;
    ssize_t ret = -EINVAL;
    
    pthread_mutex_lock(&mq->w_mutex);

    /* check current message numbers at the message queue */
    if (mq->mq_attr.mq_curmsgs >= mq->mq_attr.mq_maxmsg)
        goto check_failed;
    
    /* check the size of current message to be sent */
    if (msg_len > mq->mq_attr.mq_msgsize)
    	goto check_failed;
    
    /* get send message point */
    mq_msg = (mq_msg_t *)mq->send_ptr;
    /* check if send buffer is free */
    if (!mq_msg->used)
    {   
        /* fill the data */
        mq_msg->msg_size = msg_len;
        send_ptr = mq->send_ptr + sizeof(mq_msg_t);
        memcpy(send_ptr, msg_ptr, msg_len);
        
        /* point to next buffer */
        mq->send_ptr = mq->send_ptr + mq->mq_attr.mq_msgsize + sizeof(mq_msg_t);
        if (mq->send_ptr >= mq->mq_pbuf + mq->mq_pbuf_size)
            mq->send_ptr = mq->mq_pbuf;
        /* add the current message number */
        mq->mq_attr.mq_curmsgs++;
        
        ret = msg_len;
       
        /* make the buffer is send */
        mq_msg->used = 1;
    }
    else
    	goto check_failed;
    
    /* post a semaphore to the receive thread */
    if (!(mq->mq_attr.mq_flags & O_NONBLOCK))
        sem_post(&mq->sem);
  
    ret = 0;

check_failed:
	pthread_mutex_unlock(&mq->w_mutex);

	return ret;
}

/*
 * mq_receive - receive a message from the message queue
 *
 * @param mqdes     the handle oft he message queue
 * @param msg_ptr   receive message point
 * @param msg_len   receive message length
 * @param msg_prio  receive message priority
 *
 * @return the result
 */
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio)
{
    mq_t *mq = (mq_t *)mqdes;
    mq_msg_t *mq_msg;
    char *recv_ptr;
    ssize_t ret;
  
    pthread_mutex_lock(&mq->r_mutex);

    /* wait until receive buffer is not empty */
    if (!(mq->mq_attr.mq_flags & O_NONBLOCK))
        sem_wait(&mq->sem);
    
    /* check current message numbers at the message queue */
    if (!mq->mq_attr.mq_curmsgs)
    	goto check_failed;
    
    /* get receive message point */
    mq_msg = (mq_msg_t *)mq->recv_ptr;
    /* check if send buffer is free */
    if (mq_msg->used)
    {   
        /* fill the data */
        recv_ptr = mq->recv_ptr + sizeof(mq_msg_t);
        memcpy(msg_ptr, recv_ptr, mq_msg->msg_size);
        
        /* point to next buffer */
        mq->recv_ptr = mq->recv_ptr + mq->mq_attr.mq_msgsize + sizeof(mq_msg_t);
        if (mq->recv_ptr >= mq->mq_pbuf + mq->mq_pbuf_size)
            mq->recv_ptr = mq->mq_pbuf;
        /* add the current message number */
        mq->mq_attr.mq_curmsgs--;
        
        ret = mq_msg->msg_size;
        
        /* make the buffer is received */
        mq_msg->used = 0;
    }
    else
    	goto check_failed;
    
check_failed:
    pthread_mutex_unlock(&mq->r_mutex);

    return ret;
}

