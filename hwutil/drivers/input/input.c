/*
 * File         : input.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2020, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-03       DongHeng        create
 */

#include "input.h"
#include "errno.h"
#include "mqueue.h"

#define EVENT_TYPE_MAX                          (INPUT_KEY_FALLING_EDGE_EVENT + 1)

/* input table structure description */
struct input_event_table
{
    mqd_t           input_mqd;
};

/* input event cache table */
NO_INIT static struct input_event_table input_event_table[EVENT_TYPE_MAX];

/*
 * input_init - the function will init the input system
 *
 * @return the result 
 */
int input_init(void)
{
    int i;
    
    for (i = 0; i < EVENT_TYPE_MAX; i++)
        input_event_table[i].input_mqd = 0;
    
    return 0;
}

/*
 * input_open - the function will open the input system device
 *
 * @param type input device type
 * @param flag open device flag
 *
 * @return the result
 */
int input_open(int type, int flag)
{ 
    struct mq_attr mq_attr;
    mqd_t mqd;
    
    if (type >= EVENT_TYPE_MAX)
        return -EINVAL;
    
    mq_attr.mq_maxmsg  = 16;
    mq_attr.mq_msgsize = sizeof(struct input_event);
    
    if ((mqd = mq_open("input", flag, flag, &mq_attr)) < 0)
        return -EINVAL;
        
    input_event_table[type].input_mqd = mqd;
  
    return 0;
}

/*
 * the function will report a event to the input system
 *
 * @param type  the type of the event
 * @param event the data to be reported
 *
 * @return the result of reporting the event
 */
int input_report(os_u16 type, struct input_event *event)
{
    if (type >= EVENT_TYPE_MAX || !input_event_table[type].input_mqd)
        return -EINVAL;

    return mq_send(input_event_table[type].input_mqd, (char *)event, sizeof(struct input_event), 0);
}

/*
 * the function will catch a event through the input system
 *
 * @param type  the type of the event
 * @param data  the data to be catched
 *
 * @return the result of catching the event
 */
int input_event_catch(os_u16 type, struct input_event *event, input_flag_t flag)
{
    if (type >= EVENT_TYPE_MAX || !input_event_table[type].input_mqd)
        return -EINVAL;

    return mq_receive(input_event_table[type].input_mqd, (char *)event, sizeof(struct input_event), 0);
}

