#ifndef _ZYNQ7000_H_
#define _ZYNQ7000_H_

#include "types.h"

#define SCU_BASE                 (0xF8F00000) /* system controller unit/application processor unit */

#define PRIVATE_TIMER_BASE       (SCU_BASE + 0x00000600)


/*
 * ARM Cortex-A9 private timer description
 */
struct ptimer {
	rw_reg_t load;  //reset 0
	rw_reg_t count; //reset 0, it is a decrementing counter
	rw_reg_t ctrl;  //reset 0
	rw_reg_t isr;   //reset 0
};
#define PTIMER                   ((struct ptimer *)PRIVATE_TIMER_BASE)

/* controller register */
#define PTIMER_ENABLE_MASK       (0x00000001)
#define PTIMER_AUTO_LOAD_MASK    (0x00000002)
#define PTIMER_ENABLE_IRQ_MASK   (0x00000003)
#define PTIMER_PRESCALER_MASK    (0x0000FF00)

/* controller register */
#define PTIMER_IRQ_FLAG_MASK     (0x00000001)

#endif
