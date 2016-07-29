#include "input.h"
#include "debug.h"
#include "time.h"
#include "io.h"
#include "hal.h"

struct key_edge
{
    bool low_level;
    bool high_level;
};

NO_INIT static struct key_edge key_edge;

static void __key_init(void)
{
    GPIO_InitTypeDef  gpio_init_structure;
  
    __HAL_RCC_GPIOI_CLK_ENABLE();
   
    gpio_init_structure.Pin   = GPIO_PIN_11;
    gpio_init_structure.Mode  = GPIO_MODE_INPUT;
    gpio_init_structure.Pull  = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_HIGH; 
    HAL_GPIO_Init(GPIOI, &gpio_init_structure); 
}

static void key_timer_entry(union sigval p)
{
    if( HAL_GPIO_ReadPin( GPIOI, GPIO_PIN_11) == GPIO_PIN_SET )
    {
        key_edge.high_level = true;
    }
        
    if( true == key_edge.high_level  )
    {
        if( HAL_GPIO_ReadPin( GPIOI, GPIO_PIN_11 ) == GPIO_PIN_RESET ) 
        {
            struct input_event input_event;
            
            key_edge.high_level = false;
            input_event.value = 0;
                
            input_report(INPUT_KEY_FALLING_EDGE_EVENT, &input_event);
        }
    }
}

err_t key_configuration(void)
{
    struct sigevent sigevent;
    struct itimerspect itimerspect;
    timer_t timer;
  
    __key_init();
    
    key_edge.high_level = false;
    key_edge.low_level  = false;
    
    sigevent.sigev_notify_function = key_timer_entry;
    sigevent.sigev_value.sival_ptr = NULL;
    sigevent.sigev_notify = SIGEV_THREAD;
    timer_create(CLOCK_REALTIME, &sigevent, &timer);
    ASSERT_KERNEL(timer != 0);
    
    itimerspect.it_interval.tv_nsec = 100;
    itimerspect.it_value.tv_nsec = 100;
    timer_settime(timer, 0, &itimerspect, NULL);
    
    input_open(INPUT_KEY_FALLING_EDGE_EVENT, O_CREAT | O_RDWR);
    
    return 0;
}