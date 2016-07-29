#include "cache.h"

#include "io.h"

static void flash_art_enable(void)
{
    __HAL_FLASH_ART_ENABLE();
    
    
}

static void cpu_icache_enable(void)
{
    SCB_InvalidateICache();
	
    /* Branch prediction enable */
    SCB->CCR |= SCB_CCR_BP_Msk; 
    __DSB();

    SCB_EnableICache();   
}

static void cpu_dcache_enable(void)
{
    SCB_InvalidateDCache();
    SCB_EnableDCache();  
}

err_t cache_configuration(void)
{
    flash_art_enable();  
    cpu_icache_enable();   
    cpu_dcache_enable();
 
    return 0;
}
