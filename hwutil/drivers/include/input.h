#ifndef _INPUT_H_
#define _INPUT_H_

#include "types.h"

/* input event type */
#define INPUT_KEY_PRESS_EVENT                   0
#define INPUT_KEY_BOUNCE_EVENT                  1
#define INPUT_KEY_EDGE_EVENT                    2
#define INPUT_KEY_FALLING_EDGE_EVENT            3

typedef int                                     input_flag_t;

struct input_event
{
    int         value;
};

/* function */
int input_init(void);
int input_open(int type, int flag);
int input_report(os_u16 type, struct input_event *event);
int input_event_catch(os_u16 type, struct input_event *event, input_flag_t flag);

#endif
