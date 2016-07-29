#include "sys_tick.h"
#include "io.h"

err_t tick_configuration(void)
{
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY ,0);
    
    return 0;
}

void SysTick_Handler(void)
{
    extern void os_timetick(void);
    
    os_timetick();
}
