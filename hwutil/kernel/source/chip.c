/*
 * File         : idle.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 *
 * Change Logs:
 * DATA             Author          Note
 * 2016-05-04       DongHeng        create
 */

#include "chip.h"
#include "debug.h"
#include "stdlib.h"

/*@{*/

#define CHIP_DEBUG_LEVEL 0
#define CHIP_DEBUG_LEVEL_ENABLE 0
#define CHIP_DEBUG(level, ...) \
    if (level > CHIP_DEBUG_LEVEL) \
        printk(__VA_ARGS__);

/*@}*/
static struct chip_irq *__chip_irq;
/*@{*/

/**
 * the function will initialize all the HAL driver
 *
 * @return the result
 */
err_t chip_init(void)
{
	int order;

    for (order = 0; order < CHIP_ORDER_MAX; order++)
    {
    	int i;
    	struct chip_desc *chip = CHIP_FUNC_START_ADDR;

        for (i = 0; i < CHIP_FUNC_NUM; i++)
        {
            err_t ret;

            if (chip->order == order && (ret = chip->init()))
            {
            	CHIP_DEBUG(CHIP_DEBUG_LEVEL_ENABLE, "[%s] function initialized error,"
            		"it is for [%s], return error code %d.\r\n", chip->name, chip->desc, ret);
                while(ret);
            }
            chip++;
        }
    }
  
    return 0;
}

/**
 * chip_irq_handle - the function will handle the interrupt service, it will be called
 *                   by system hardware interrupt entry
 *
 * @return none
 */
void chip_irq_handle(void)
{
	size_t irq;
	struct chip_irq *chip_irq = __chip_irq;
	struct os_isr_desc *isr_table = chip_irq->isr_table;

	irq = chip_irq->request();

	if (isr_table[irq].handle)
		isr_table[irq].handle(irq, isr_table[irq].param);

	if (chip_irq->clear)
	    chip_irq->clear(irq);
}

err_t chip_irq_register(struct chip_irq *chip_irq)
{
	struct os_isr_desc *isr_table;

	ASSERT_KERNEL(chip_irq);
	ASSERT_KERNEL(chip_irq->num);

	isr_table = malloc(sizeof(struct os_isr_desc) * chip_irq->num);
	ASSERT_KERNEL(isr_table);
	chip_irq->isr_table = isr_table;

	__chip_irq = chip_irq;

	return 0;
}

/*@}*/
