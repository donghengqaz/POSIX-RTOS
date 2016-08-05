#include "ptimer.h"
#include "zynq7000.h"

/* definition for private timer function */
#define PTIMER_CLEAR_IRQ_FLAG()     PTIMER->isr = PTIMER_IRQ_FLAG_MASK
#define PTIMER_DISABLE()            PTIMER->ctrl &= PTIMER_ENABLE_MASK
#define PTIMER_ENABLE()             PTIMER->ctrl |= PTIMER_ENABLE_MASK
#define PTIMER_SET_LOAD(x)          PTIMER->load = x; PTIMER->count = x
#define PTIMER_ENABLE_AUTOLOAD()    PTIMER->ctrl |= PTIMER_AUTO_LOAD_MASK
#define PTIMER_ENABLE_IRQ()         PTIMER->ctrl |= PTIMER_ENABLE_IRQ_MASK

#define PTIMER_DEINIT()             PTIMER->ctrl = 0;

/* user private timer function point */
static void (*ptimer_isr)(int vect, void *p);

/* hardware private timer function point, set it here so that user need not to clear the flag */
static void hw_ptimer_isr(int vect, void *p)
{
    ptimer_isr(vect, p);

    PTIMER_CLEAR_IRQ_FLAG();
}

static void ptimer_deinit(void)
{
	PTIMER_DEINIT();
}

int ptimer_init(struct ptimer_param *param)
{
    if (!param || !param->period)
        return -1;

    /* clear all private timer hardware configuration */
    ptimer_deinit();

    /* the private timer clock frequency is 1/2 APU clock (CPU3x2x) */
    PTIMER_SET_LOAD(APU_FREQ / 2 / param->period);

    /* set the reload function */
    if (true == param->auto_reload)
        PTIMER_ENABLE_AUTOLOAD();

    /* register the user interrupt server */
    if (param->isr)
    {
        ptimer_isr = param->isr;
        PTIMER_ENABLE_IRQ();

        PTIMER_CLEAR_IRQ_FLAG();

        /* register and enable the interrupt into the core */
    }

    /* enable the private timer */
    if (true == param->start)
        PTIMER_ENABLE();

    return 0;
}

void ptimer_start(bool enable)
{
    if (true == enable)
        PTIMER_ENABLE();
    else
        PTIMER_DISABLE();
}


