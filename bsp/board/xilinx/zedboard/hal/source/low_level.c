
#include "stdio.h"

size_t low_level_init(void)
{
    uart_configuration();
    
    printk("low level init ok.\r\n");
  
    return (0);
}

void hw_interrupt_disable(void)
{

}
