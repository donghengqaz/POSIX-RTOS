
#include "stdio.h"

size_t low_level_init(void)
{
    
    printk("low level init ok.\r\n");
  
    return (0);
}

int fputc(int ch, FILE *stream)
{
    return 0;
}
