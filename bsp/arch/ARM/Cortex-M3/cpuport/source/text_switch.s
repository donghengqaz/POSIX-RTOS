;/**
; * @addtogroup cohzex-m3
; */
;/*@{*/

SCB_VTOR        EQU     0xE000ED08               ; Vector Table Offset Register
NVIC_INT_CTRL   EQU     0xE000ED04               ; interrupt control state register
NVIC_SYSPRI2    EQU     0xE000ED20               ; system priority register (2)
NVIC_SYSPRI4    EQU     0xE000ED20               ; system priority register (4)
NVIC_PENDSV_PRI EQU     0x00FF0000               ; PendSV priority value (lowest)
NVIC_PENDSV_PI  EQU     0x000000ff               ; PendSV priority value (lowest)
NVIC_PENDSVSET  EQU     0x10000000               ; value to trigger PendSV exception

    SECTION    .text:CODE(2)
    THUMB
    REQUIRE8
    PRESERVE8

    IMPORT hz_thread_switch_interrupt_flag
    IMPORT hz_interrupt_from_thread
    IMPORT hz_interrupt_to_thread
#ifdef USING_KERNEL_SECTION
    IMPORT hz_scheduler_privilege_mode
#endif

    EXPORT hz_hw_context_switch_interrupt
    EXPORT hz_hw_context_switch
hz_hw_context_switch_interrupt:
hz_hw_context_switch:
    ; set hz_thread_switch_interrupt_flag to 1
    LDR     r2, =hz_thread_switch_interrupt_flag
    LDR     r3, [r2]
    CMP     r3, #1
    BEQ     _reswitch
    MOV     r3, #1
    STR     r3, [r2]

    LDR     r2, =hz_interrupt_from_thread   ; set hz_interrupt_from_thread
    STR     r0, [r2]

_reswitch
    LDR     r2, =hz_interrupt_to_thread     ; set hz_interrupt_to_thread
    STR     r1, [r2]

    ; set the PendSV exception priority
    LDR     r0, =NVIC_SYSPRI2
    LDR     r1, =NVIC_PENDSV_PRI
    LDR.W   r2, [r0,#0x00]       ; read
    ORR     r1,r1,r2             ; modify
    STR     r1, [r0]             ; write-back

    LDR     r0, =NVIC_INT_CTRL              ; trigger the PendSV exception (causes context switch)
    LDR     r1, =NVIC_PENDSVSET
    STR     r1, [r0]
    BX      LR

; r0 --> switch from thread stack
; r1 --> switch to thread stack
; psr, pc, lr, r12, r3, r2, r1, r0 are pushed into [from] stack
    EXPORT PendSV_Handler
PendSV_Handler:

    ; disable interrupt to protect context switch
    MRS     r2, PRIMASK
    CPSID   I

    ; get hz_thread_switch_interrupt_flag
    LDR     r0, =hz_thread_switch_interrupt_flag
    LDR     r1, [r0]
    CBZ     r1, pendsv_exit         ; pendsv already handled

    ; clear hz_thread_switch_interrupt_flag to 0
    MOV     r1, #0x00
    STR     r1, [r0]

    LDR     r0, =hz_interrupt_from_thread
    LDR     r1, [r0]
    CBZ     r1, switch_to_thread    ; skip register save at the first time

    MRS     r1, psp                 ; get from thread stack pointer
    STMFD   r1!, {r4 - r11}         ; push r4 - r11 register
    LDR     r0, [r0]
    STR     r1, [r0]                ; update from thread stack pointer

switch_to_thread
    LDR     r1, =hz_interrupt_to_thread
    LDR     r1, [r1]
    LDR     r1, [r1]                ; load thread stack pointer

    LDMFD   r1!, {r4 - r11}         ; pop r4 - r11 register
#ifdef USING_KERNEL_SECTION
    LDR     r3, =hz_scheduler_privilege_mode
    LDR     r3, [r3]
    CBZ     r3, cpu_switch_to_usr
    ;BIC     LR, LR, #0x0F
    ;ORR     LR, LR, #0x09
    ;MSR     msp, r1
    MSR     psp, r1
    MRS     r3, CONTROL
    BIC     r3, r3, #0x01
    MSR     CONTROL, r3  
    b      pendsv_exit
cpu_switch_to_usr
    ;BIC     LR, LR, #0x0F
    ;ORR     LR, LR, #0x0D
    MRS     r3, CONTROL
    ORR     r3, r3, #0x01
    MSR     CONTROL, r3
#endif
    MSR     psp, r1                 ; update stack pointer


pendsv_exit
    ; restore interrupt  
    MSR     PRIMASK, r2
#ifndef USING_KERNEL_SECTION
    ORR     lr, lr, #0x04
#endif
    BX      lr

;/*
; * void hz_hw_context_switch_to(hz_uint32 to);
; * r0 --> to
; */
    EXPORT hz_hw_context_switch_to
hz_hw_context_switch_to:
    LDR     r1, =hz_interrupt_to_thread
    STR     r0, [r1]

    ; set from thread to 0
    LDR     r1, =hz_interrupt_from_thread
    MOV     r0, #0x0
    STR     r0, [r1]

    ; set interrupt flag to 1
    LDR     r1, =hz_thread_switch_interrupt_flag
    MOV     r0, #1
    STR     r0, [r1]

    ; set the PendSV exception priority
    LDR     r0, =NVIC_SYSPRI2
    LDR     r1, =NVIC_PENDSV_PRI
    LDR.W   r2, [r0,#0x00]       ; read
    ORR     r1,r1,r2             ; modify
    STR     r1, [r0]             ; write-back

    LDR     r0, =NVIC_INT_CTRL      ; trigger the PendSV exception (causes context switch)
    LDR     r1, =NVIC_PENDSVSET
    STR     r1, [r0]

    ; restore MSP
    LDR     r0, =SCB_VTOR
    LDR     r0, [r0]
    LDR     r0, [r0]
    NOP

    CPSIE   I                       ; enable interrupts at processor level
    NOP                             ; delay a clock
    ; never reach here!

; compatible with old version
    EXPORT hz_hw_interrupt_thread_switch
hz_hw_interrupt_thread_switch:
    BX      lr
    
    EXPORT hz_scheduler_active_privilege
hz_scheduler_active_privilege
    MRS     r0, IPSR
    CBZ     r0, call_svc
    BX      LR
call_svc
    SVC     #32
    BX      LR
    
    EXPORT SVC_Handler
SVC_Handler
    MRS     r3, CONTROL
    BIC     r3, r3, #0x01
    MSR     CONTROL, r3
    BX      lr
    
    EXPORT hz_cpu_state
hz_cpu_state
    MRS     r0, CONTROL
    BX      lr
   
    END
