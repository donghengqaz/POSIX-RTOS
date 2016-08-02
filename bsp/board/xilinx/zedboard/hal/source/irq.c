#include "rtos.h"

/*!< 4 bits for pre-emption priority
     0 bits for subpriority */

err_t irq_configuration(void)
{
 
    return 0;
}

void hw_context_switch_to(phys_reg_t to)
{

}
void hw_context_switch(int from, int to)
{

}
void hw_interrupt_recover(phys_reg_t a)
{

}
phys_reg_t hw_interrupt_suspend(void)
{
	return 0;
}
int pthread_hw_stack_init(void* a, void*b, void*c, void*d)
{
	return 0;
}

void hw_interrupt_disable(void)
{

}
