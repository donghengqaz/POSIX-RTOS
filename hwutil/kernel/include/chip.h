#ifndef _HAL_H_
#define _HAL_H_

#include "init.h"

/* chip system description */
struct chip_desc
{
#define CHIP_ORDER_MAX       16
  
    const char        *name;
    const char        *desc;
    err_t             (*init) (void);
    size_t            order;
};

#define CHIP_DESC_EXPORT(func, desc, order)  \
    static const char __chip##_##func##_func[] = #func;  \
    static const char __chip##_##func##_desc[] = #desc;  \
    const struct chip_desc __chip##_##func __CHIP_SECTION = \
    { \
        __chip##_##func##_func, \
        __chip##_##func##_desc, \
        func, \
        order \
    };

err_t chip_init(void);

/* chip interrupt description */

/* description for system hardware interrupt */
typedef int (*os_isr_handle_t)(int vect, void *param);

struct os_isr_desc{
	/* the IRQ function handle */
	os_isr_handle_t         handle;
	/* the IRQ parameter */
	void                    *param;
};

struct chip_irq
{
	/* the maxim number of the IRQ */
	size_t                   num;

	/* get the IRQ number is to be handled */
	size_t                   (*request)(void);

	/* mask the IRQ and mode it work */
	err_t                    (*mask)(size_t irq);
	/* unmask the IRQ and mode it not work */
	err_t                    (*unmask)(size_t irq);

	/* clear the flag of the IRQ which is done */
	err_t                    (*clear)(size_t irq);

	/* the IRQ table */
	struct os_isr_desc       *isr_table;
};

err_t chip_irq_register(struct chip_irq *chip_irq);

#endif
