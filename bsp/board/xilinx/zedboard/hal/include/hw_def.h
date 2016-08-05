#ifndef _HW_DEF_H_
#define _HW_DEF_H_

#include "types.h"

#define APU_FREQ (833 * 1000 * 1000) // MHz

/* systick irq cycle time (millisecond) */
#define RTOS_SYS_TICK_PERIOD 1UL

/* the end address of the ram used by heap */
#define HW_RAM_ADDR_ADDR (0x100000UL + 0x1FF00000UL)

#define STLINK_VCP_COM

/* rtos function definition */
#define USING_SHELL 0
#define USING_IPPORT 0

/* disable the C lib */
#define USING_MALLOC_LIB 0
#define USING_STRING_LIB 0
#define USING_MALLOC_LIB 0

/* define the hardware bytes align */
#define HW_ALIGN_SIZE 4

#define HW_ETH_RX_BUFFER_NUM_MAX 4
#define HW_ETH_RX_BUFFER_LENGTH_MAX 1560

/* { */

/* } */

#endif
