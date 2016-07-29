#include "chip_test.h"

#include "io.h"

static void test_func(void)
{
    volatile int i;
  
    for(i = 0; i < 5000; i++)
    {
        ;
    }
}

os_u32 chip_test(void)
{
    os_u32 start_tick = SysTick->VAL;
    os_u32 end_tick;
    
    test_func();
    
    end_tick = SysTick->VAL;
    
    return end_tick - start_tick;
}

/*
 * core-M7 : time = 192332 - 182300 = 10032
 * core-M3 : time = 359961 - 294933 = 65028
 */