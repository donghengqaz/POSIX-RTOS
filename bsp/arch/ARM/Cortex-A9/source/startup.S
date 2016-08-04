

.equ MODE_USR,		0x10
.equ MODE_FIQ,		0x11
.equ MODE_IRQ,		0x12
.equ MODE_SVC,		0x13
.equ MODE_ABT,		0x17
.equ MODE_UND,		0x1B
.equ MODE_SYS,		0x1F

.equ I_BIT,			0x80		/* when I bit is '1', disable IRQ */
.equ F_BIT,			0x40		/* when F bit is '1', disable FIQ */

.equ UND_STACK_SIZE,	0x00000000
.equ ABT_STACK_SIZE,	0x00000000
.equ FIQ_STACK_SIZE,	0x00000000
.equ UND_STACK_SIZE,	0x00000100
.equ IRQ_STACK_SIZE,	0x00000100
.equ USR_STACK_SIZE,	0x00000000

#define STACK_SIZE_TOTAL (UND_STACK_SIZE + ABT_STACK_SIZE + FIQ_STACK_SIZE + UND_STACK_SIZE + \
						  IRQ_STACK_SIZE + USR_STACK_SIZE)

.global stack_top

.section .bss
stack_top:

.section .test
/* chip reset entry */
.globl _reset
_reset:
	/* disable SCU */
    ldr r7, =0xF8F0000C
    ldr r6, =0x0000FFFF
    str r6, [r7]

    /* disable MMU */
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #0x1
    mcr p15, 0, r0, c1, c0, 0

    /* disable the interrupt and switch to SVC32 mode */
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #0x13
    msr cpsr_c, r0

    /* setup stack */
    bl setup_stack

    /* initialize .bss */
    mov r0, #0x0
    ldr r1, =__bss_start
    ldr r2, =__bss_end

init_bss_loop:
    cmp r1, r2
    strlo r0, [r1], #4
    blo init_bss_loop

    /* initialize .data, move data from data to sdata seaction */
    ldr r1, =__sdata_start
    ldr r2, =__sdata_end
    ldr r3, =__data_start

init_data_loop:
    cmp r1, r2
    ldrlo r0, [r3], #4
    strlo r0, [r1], #4
    blo init_data_loop

    /* jump to main */
    ldr pc, _main

_main:
    .word main

setup_stack:
    ldr r0, =stack_top

    /* setup stack for svc */
    mov sp, r0

    /* enter the undefined Instruction mode and set stack for it */
    msr cpsr_c, #MODE_UND | I_BIT | F_BIT
    mov sp, r0
    /* update the stack point */
    sub r0, r0, #UND_STACK_SIZE

    /* enter the abort mode and set stack for it */
    msr cpsr_c, #MODE_ABT | I_BIT | F_BIT
    mov sp, r0
    /* update the stack point */
    sub r0, r0, #ABT_STACK_SIZE

    /* enter the FIQ mode and set stack for it */
    msr cpsr_c, #MODE_FIQ | I_BIT | F_BIT
    mov sp, r0
    /* update the stack point */
    sub r0, r0, #FIQ_STACK_SIZE

    /* enter the IRQ mode and set stack for it */
    msr cpsr_c, #MODE_IRQ | I_BIT | F_BIT
    mov sp, r0
    /* update the stack point */
    sub r0, r0, #IRQ_STACK_SIZE

    /* switch to SVC32 mode */
    msr cpsr_c, #MODE_SVC | I_BIT | F_BIT

    bx lr


