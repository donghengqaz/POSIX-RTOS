#include "sys_tick.h"

#include "io.h"

#include "time.h"

#define ONE_SENCOND (1000)

#define HAL_SYS_TIMER_PERIODIC (5)

err_t tick_confugration(void)
{
    SysTick_Config( SystemCoreClock / (ONE_SENCOND / HAL_SYS_TIMER_PERIODIC) );
  
    return 0;
}

size_t low_level_period(void)
{
    return HAL_SYS_TIMER_PERIODIC;
}

size_t tick_read(void)
{
    return SysTick->VAL;
}

extern void SysTick_Handler(void)
{
    timer_poll();
}
