#include "irq.h"

#include "io.h"

/*!< 4 bits for pre-emption priority
     0 bits for subpriority */

err_t irq_configuration(void)
{
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
 
    return 0;
}