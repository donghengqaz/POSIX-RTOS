#ifndef _HWUTIL_H_
#define _HWUTIL_H_

#include "hw_def.h"

/*
 *
 */
#define ALIGN(size)                     __ALIGN(size, HW_ALIGN_SIZE)
/******************************************************************************/

/*
 *
 */
#define ITOC_RS0(x)                     (char)(x >> 0)
#define ITOC_RS8(x)                     (char)(x >> 8)
#define ITOC_RS16(x)                    (char)(x >> 16)
#define ITOC_RS24(x)                    (char)(x >> 24)
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

#define PHYS_ADDR_ALIGN(addr)           (ALIGN((phys_addr_t)addr) == (phys_addr_t)addr)

#if USING_KERNEL_MEMORY_ISOLATION
    #define __user
    #define __kernel
#else
    #define __user
    #define __kernel    
#endif

#if USING_KERNEL_SECTION
    #define KERNEL_SECTION SECTION("kernel_rw")
#else
    #define KERNEL_SECTION 
#endif

#ifndef hw_interrupt_suspend
    extern phys_reg_t hw_interrupt_suspend(void);
#endif 

#ifndef hw_interrupt_recover
    extern void hw_interrupt_recover(phys_reg_t temp);
#endif
    
#ifndef SCHED_CYCLE
    #define SCHED_PERIOD 1000
#endif

#endif
