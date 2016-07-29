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
 * all                            : time = 192332 - 182300 = 10032
 * no art & icache & dcache       : time = 192434 - 182405 = 10029
 * art & no icache & dcache       : time = 187320 - 177257 = 10063
 * no dcache & icache & no art    : time = 195981 - 185942 = 10039
 * no icache & no dcache & no art : time = 193964 - 068880 = 125084
 */