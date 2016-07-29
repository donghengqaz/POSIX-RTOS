/*
 * File         : led_trigger.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-02       DongHeng        create the first version
 */

#include "led_trigger.h"  
   
#include "errno.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"

#include "debug.h"
#include "sched.h"
  
/*@{*/
#define HEART_TRIGGER_TIME                      50      /* ms */ 
#define HEART_TRIGGER_CYCLE                     2000    /* ms */ 
#define HEART_TRIGGER_COUNT                     (HEART_TRIGGER_CYCLE / HEART_TRIGGER_TIME)

#define EVENT_TRIGGER_TIME                      50      /* ms */ 
   
/*@}*/
  
/*@{*/ 

/**
 * This function is the entry function of heart trigger timer timeout
 *
 * @param p no meaning
 */
static void led_heart_trigger_entry(union sigval p)
{
    struct led_heart_trigger *led_heart_trigger = (struct led_heart_trigger *)p.sival_ptr;
    const os_u16 led_trigger_speed[] = {  2,  6, 10, 14, 18, 22, 24, 28, 32, 36, 40 };
    const os_u16 led_trigger_total[] = { 39, 35, 39, 27, 35, 43, 47, 27, 31, 35, 39 };
    
    if( 0 == led_heart_trigger->led_count )
    {
        led_heart_trigger->led_on();
    }
    else
    {
        led_heart_trigger->led_off();
    }
  
    led_heart_trigger->led_count = ( led_heart_trigger->led_count + 1 ) % led_heart_trigger->led_cycle;
    
    led_heart_trigger->trigger_count++;
    
    if( led_heart_trigger->trigger_count >= led_heart_trigger->trigger_total )
    {
        os_u16 speed_level = 10 - get_current_usage() / 10 % 11;
        
        led_heart_trigger->led_cycle = led_trigger_speed[speed_level];
        led_heart_trigger->trigger_total = led_trigger_total[speed_level];
        
        led_heart_trigger->trigger_count = 0;
        led_heart_trigger->led_count = 0;
    }
}

/*
 * the function will init a heart trigger led function
 *
 * @param trigger the point of the heart trigger data
 *
 * @return the result of initing the trigger
 */
err_t led_heart_trigger_init(struct led_heart_trigger *trigger)
{
    struct led_heart_trigger *led_heart_trigger;
    struct sigevent sigevent;
    struct itimerspect itimerspect;
    timer_t timer;
    
    if (!(led_heart_trigger = malloc(sizeof(struct led_heart_trigger))))
        return -EINVAL;
   
    memcpy(led_heart_trigger, trigger, sizeof(struct led_heart_trigger));
    led_heart_trigger->led_cycle = HEART_TRIGGER_COUNT;
    led_heart_trigger->trigger_total = HEART_TRIGGER_COUNT;
    
    /* create a timer for hear-trigger running in time */
    sigevent.sigev_notify_function = led_heart_trigger_entry;
    sigevent.sigev_value.sival_ptr = led_heart_trigger;
    sigevent.sigev_notify = SIGEV_THREAD;
    timer_create(CLOCK_REALTIME, &sigevent, &timer);    
    ASSERT_KERNEL(timer != 0);
    
    itimerspect.it_interval.tv_nsec = HEART_TRIGGER_TIME;
    itimerspect.it_value.tv_nsec = HEART_TRIGGER_TIME;
    timer_settime(timer, 0, &itimerspect, NULL);
  
    return 0;
}

/**
 * This function is the entry function of heart trigger timer timeout
 *
 * @param p no meaning
 */
static void led_event_trigger_entry(union sigval p)
{
    struct led_event_trigger *led_event_trigger = (struct led_event_trigger *)p.sival_ptr;
    os_u32 data;
  
    if (!queue_pop(led_event_trigger->queue, &data))
    {
        led_event_trigger->led_on();
        led_event_trigger->state = 1;
    }
    else if (1 == led_event_trigger->state)
    {
        led_event_trigger->state = 2;
    }
    else
    {
        led_event_trigger->state = 0;
        led_event_trigger->led_off();
    }
}

/*
 * event_trigger_init - the function will init a event trigger led function
 *
 * @param trigger the point of the heart trigger data
 *
 * @return the result of initing the trigger
 */
err_t led_event_trigger_init(struct led_event_trigger *trigger)
{
    struct led_event_trigger *led_event_trigger;
    struct sigevent sigevent;
    struct itimerspect itimerspect;
    timer_t timer;
    
    if (!trigger->led_off || !trigger->led_on || !trigger->queue)
        return -EINVAL;
    
    if (!(led_event_trigger = malloc(sizeof(struct led_event_trigger))))
        return -EINVAL;
   
    memcpy(led_event_trigger, trigger, sizeof(struct led_event_trigger));
    led_event_trigger->state = 0;
    
    /* create a timer for hear-trigger running in time */
    sigevent.sigev_notify_function = led_event_trigger_entry;
    sigevent.sigev_value.sival_ptr = led_event_trigger;
    sigevent.sigev_notify = SIGEV_THREAD;
    timer_create(CLOCK_REALTIME, &sigevent, &timer);    
    ASSERT_KERNEL(timer != 0);
    
    itimerspect.it_interval.tv_nsec = EVENT_TRIGGER_TIME;
    itimerspect.it_value.tv_nsec = EVENT_TRIGGER_TIME;
    timer_settime(timer, 0, &itimerspect, NULL);
  
    return 0;
}

/*@}*/
