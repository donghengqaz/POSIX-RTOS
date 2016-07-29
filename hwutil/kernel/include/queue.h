#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "types.h"
#include "errno.h"

/*  */
#define QUEUE_BUFFER_MAX             32

/*  */
struct queue
{
    os_u32              buffer[QUEUE_BUFFER_MAX];
    os_u16              head;
    os_u16              tail;
};
typedef struct queue queue_t;

/*
 * the function will inser data to the queue
 *
 * param queue the point of the queue
 * param data the data to be insert
 *
 * return the result of inserting data
 *
 */
INLINE err_t queue_push(queue_t *queue, os_u32 data)
{
    os_u16 point = (queue->head + 1) % QUEUE_BUFFER_MAX;
    
    if( point != queue->tail )
    {
        queue->buffer[queue->head] = data;
        queue->head = point;
        
        return 0;
    }
    
    return -1;
}

/*
 * the function will take data from the queue
 *
 * param queue the point of the queue
 * param data the point of the data buffer
 *
 * return the result of taking data
 *
 */
INLINE err_t queue_pop(queue_t *queue, os_u32 *data)
{   
    if( queue->head != queue->tail )
    {
        *data = queue->buffer[queue->tail];
        queue->tail= (queue->tail + 1) % QUEUE_BUFFER_MAX;
        
        return 0;
    }
    
    return -1;
}

/*
 * the function will init queue
 */
INLINE void queue_init(queue_t *queue)
{
    queue->tail = queue->head = 0;
}

 
#endif
