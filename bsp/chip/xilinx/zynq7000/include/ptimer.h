#ifndef _PTIMER_H_
#define _PTIMER_H_

#include "rtos.h"

struct ptimer_param{
    /* the private timer cycle period */
    os_u32 period;

    /* set if the private timer is auto-load mode */
    bool   auto_reload;

    /* set the private timer interrupt server */
    void   (*isr)(int vect, void *p);

    /* set if it start the timer when it finish initializing the timer */
    bool   start;
};

int ptimer_init(struct ptimer_param *param);
void ptimer_start(bool enable);

#endif
