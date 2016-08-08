#include "rtos.h"
#include "chip.h"

/* mask for ARM Cortex-A9 interrupt description */

static size_t interrupt_request(void)
{


    return 0;
}

static err_t interrupt_clear(size_t irq)
{
    return 0;
}

static int interrupt_init(void)
{
    static struct chip_irq chip_irq;

    extern os_u32 _vector_table;
    extern void set_vector(int vect);

    set_vector(_vector_table);

    chip_irq.num     = CHIP_MAX_IRQ_NUM;
    chip_irq.request = interrupt_request;
    chip_irq.clear   = interrupt_clear;
    chip_irq_register(&chip_irq);

    return 0;
}
CHIP_DESC_EXPORT(interrupt_init, "ARM Cortex-A9 IRQ", 0);
