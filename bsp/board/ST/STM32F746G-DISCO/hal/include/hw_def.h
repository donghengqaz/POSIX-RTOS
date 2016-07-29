#ifndef _HW_DEF_H_
#define _HW_DEF_H_

#include "types.h"

/* systick irq cycle time (millisecond) */
#define RTOS_SYS_TICK_PERIOD 1UL

/* the end address of the ram used by heap */
#define HW_RAM_ADDR_ADDR 0x2004BFFFUL

#define STLINK_VCP_COM

/* rtos function definition */
#define USING_SHELL 1
#define USING_IPPORT 1

/* disable the C lib */
#define USING_MALLOC_LIB 0
#define USING_STRING_LIB 0
#define USING_MALLOC_LIB 0

/* define the hardware bytes align */
#define HW_ALIGN_SIZE 4

#define HW_ETH_RX_BUFFER_NUM_MAX 4
#define HW_ETH_RX_BUFFER_LENGTH_MAX 1560

#endif
