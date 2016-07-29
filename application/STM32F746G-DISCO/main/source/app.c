#include "pthread.h"
#include "input.h"
#include "stdio.h"
#include "debug.h"

static void* app_entry(void *p)
{
    struct input_event input_event;
  
    while (1)
    {
        if (input_event_catch(INPUT_KEY_FALLING_EDGE_EVENT, &input_event, 0) > 0)
        {
            printf("key [%d] inpressed.\r\n", input_event.value + 1);
        }
    }
}


err_t app_init(void)
{  
    int err;
    int tid;
    
    err = pthread_create(&tid,
                         NULL,
                         app_entry,
                         NULL);
    ASSERT_KERNEL(!err);
  
    return 0;
}