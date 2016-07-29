/*
 * File         : init.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-11       DongHeng        create
 */

#include "init.h"


#include "sched.h"
#include "debug.h"
#include "errno.h"
#include "idle.h"
#include "input.h"
#include "stdlib.h"
#include "signal.h"
   
#include "ipport.h"
#include "shell.h"
#include "time.h"

/*@{*/ 

#define RTOS_INTERRUPT_DISABLE                                              \
{                                                                              \
    extern void hw_interrupt_disable(void);                                 \
    hw_interrupt_disable();                                                 \
}

#define RTOS_INTERRUPT_ENABLE                                               \
{                                                                              \
    extern void hw_interrupt_enable(void);                                  \
    hw_interrupt_enable();                                                  \
}

/*@}*/

/*@{*/

#ifdef __CC_ARM                                                                
extern int Image$$RW_IRAM1$$ZI$$Limit;                                         
#elif __ICCARM__                                                               
#pragma section="HEAP"                                                         
#else                                                                          
extern int __bss_end;                                                          
#endif                                                                         
                                                                               
#ifdef __CC_ARM                                                                
#define HEAP_MEM_INIT()    heap_mem_init((size_t)&Image$$RW_IRAM1$$ZI$$Limit, HW_RAM_ADDR_ADDR)
#elif __ICCARM__                                                               
#define HEAP_MEM_INIT()    heap_mem_init((size_t)__segment_end("HEAP"), HW_RAM_ADDR_ADDR)
#else                                                                          
#define HEAP_MEM_INIT()    heap_mem_init((size_t)&__bss_end, HW_RAM_ADDR_ADDR)
#endif

/*@}*/

/*@{*/ 

/*
 * the function will init the lowlevel for the kernel starting
 */
static void rtos_low_level_init(void)
{
    extern err_t low_level_init(void);
    
    ASSERT_KERNEL(!low_level_init());
}

/*
 * the function will initialize the driver for kernel and device
 */
static void rtos_driver_init(void)
{
    ASSERT_KERNEL(!input_init());
#if USING_DISK_PORT
    ASSERT_KERNEL(!diskio_port_init());
#endif
}

/*
 * the function will init the kernel
 */
static void rtos_kernel_init(void)
{
    extern int stdobj_init(void);
    extern void heap_mem_init(phys_addr_t begin_addr, phys_addr_t end_addr);
  
    HEAP_MEM_INIT();
    
    sched_init();
    signal_init();

    ASSERT_KERNEL(!timer_init());
    ASSERT_KERNEL(!stdobj_init());
         
    ASSERT_KERNEL(!shell_init());
    ASSERT_KERNEL(!ipport_system_init());

    rtos_driver_init();
}

/*
 * the function will initialize the HAL
 */
static void rtos_hal_init(void)
{
    extern err_t hal_init(void);
    
    ASSERT_KERNEL(!hal_init());
}

/*
 * the function will initialize the idle thread
 */
static void rtos_idle_init(void)
{   
    ASSERT_KERNEL(!idle_thread_init()); 
}

/*
 * the function will initialize the application program
 */
static void rtos_app_init(void)
{
    extern err_t app_init(void);
    
    ASSERT_KERNEL(!app_init());
}

/*
 * the function will start the POSIX-RTOS
 */
static void rtos_start(void)
{
    sched_start();
}

/*
 * the function is for user starting the POSIX-RTOS
 */
void rtos_run(void)
{   
    RTOS_INTERRUPT_DISABLE;
    
    rtos_low_level_init();
    
    rtos_kernel_init();
    
    rtos_hal_init();
    
    rtos_idle_init();
    
    rtos_app_init();
    
    rtos_start();
}

/*@}*/
