
.section .vectors

.global _vector_table
_vector_table:
    ldr pc, _vector_reset
    ldr pc, _vector_undef
    ldr pc, _vector_swi
    ldr pc, _vector_pabt
    ldr pc, _vector_dabt
    ldr pc, _vector_resv
    ldr pc, _vector_irq
    ldr pc, _vector_fiq

.global _reset_handle
.global _undef_handle
.global _swi_handle
.global _pabt_handle
.global _dabt_handle
.global _resv_handle
.global _irq_handle
.global _fiq_handle

_vector_reset:
    .word _reset_handle
_vector_undef:
    .word _undef_handle
_vector_swi:
    .word _swi_handle
_vector_pabt:
    .word _pabt_handle
_vector_dabt:
    .word _dabt_handle
_vector_resv:
    .word _resv_handle
_vector_irq:
    .word _irq_handle
_vector_fiq:
    .word _fiq_handle

.balignl 16, 0xDEADBEEF

.section .text.irq

    .align  5
_reset_handle:
    b _reset

_fiq_handle:
    /* FIQ mode has private r8-r12 register */
    stmfd sp!, {r0-r7, lr}
    bl    chip_irq_handle
    ldmfd sp!, {r0-r7, lr}
    subs  pc, lr, #0x04

_irq_handle:
    stmfd sp!, {r0-r12, lr}
    bl    chip_irq_handle
    ldmfd sp!, {r0-r12, lr}
    subs  pc, lr, #0x04

_undef_handle:
    subs pc, lr, #0x04

_swi_handle:
    subs pc, lr, #0x04

_pabt_handle:
    subs pc, lr, #0x04

_dabt_handle:
    subs pc, lr, #0x04

_resv_handle:
    subs pc, lr, #0x04

.section .text

.global set_vector
 set_vector:
    mrc p15, #0, r1, c1, c0, #0
    bic r1,  #(0x01 << 13)
    mcr p15, #0, r1, c1, c0, #0

    mcr p15, #0, r0, c12, c0, #0

    bx  lr
