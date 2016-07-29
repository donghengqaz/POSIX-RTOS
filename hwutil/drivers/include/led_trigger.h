#ifndef _HEART_TRIGGER_H_
#define _HEART_TRIGGER_H_

#include "types.h"
#include "queue.h"

/*@{*/

/*
 * heart-trigger structure dscription
 */
struct led_heart_trigger
{
    void (*led_on) (void);
    void (*led_off)(void);

    os_u16 led_count;
    os_u16 led_cycle;
    
    os_u16 trigger_count;
    os_u16 trigger_total;
};

/*
 *
 */
struct led_event_trigger
{
    void (*led_on) (void);
    void (*led_off)(void);

    os_u16      state;
    
    queue_t     *queue;
};

/*@}*/

/*@{*/

err_t led_heart_trigger_init(struct led_heart_trigger *trigger);
err_t led_event_trigger_init(struct led_event_trigger *trigger);

/*@}*/

#endif
