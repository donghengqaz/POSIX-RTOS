/*
 * File         : idle.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 *
 * Change Logs:
 * DATA             Author          Note
 * 2016-05-04       DongHeng        create
 */

#include "hal.h"
#include "debug.h"
#include "init.h"

/*@{*/

#define HAL_DEBUG_LEVEL 0
#define HAL_DEBUG_LEVEL_ENABLE 0
#define HAL_DEBUG(level, ...) \
    if (level > HAL_DEBUG_LEVEL) \
        printk(__VA_ARGS__);

/*@}*/

/*@{*/

/**
 * the function will initialize all the HAL driver
 *
 * @return the result
 */
err_t hal_init(void)
{
    for (int order = 0; order < HAL_ORDER_MAX; order++)
    {
        hal_func_t *hal_func = HAL_FUNC_START_ADDR;
        
        for (int i = 0; i < HAL_FUNC_NUM; i++)
        {
            err_t ret;
        
            if (hal_func->order == order && (ret = hal_func->init()))
            {
            	HAL_DEBUG(HAL_DEBUG_LEVEL_ENABLE, "[%s] function initialized error,"
            		"it is for [%s], return error code %d.\r\n", hal_func->name, hal_func->desc, ret);
                while(ret);
            }
            hal_func++;
        }
    }
  
    return 0;
}

/*@}*/
