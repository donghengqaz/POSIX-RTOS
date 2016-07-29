;/**
; * @addtogroup cortex-m7
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

    IMPORT thread_switch_interrupt_flag
    IMPORT interrupt_from_thread
    IMPORT interrupt_to_thread

    EXPORT hw_context_switch_interrupt
    EXPORT hw_context_switch
hw_context_switch_interrupt:
hw_context_switch:
    ; set thread_switch_interrupt_flag to 1
    LDR     r2, =thread_switch_interrupt_flag
    LDR     r3, [r2]
    CMP     r3, #1
    BEQ     _reswitch
    MOV     r3, #1
    STR     r3, [r2]

    LDR     r2, =interrupt_from_thread   ; set interrupt_from_thread
    STR     r0, [r2]

_reswitch
    LDR     r2, =interrupt_to_thread     ; set interrupt_to_thread
    STR     r1, [r2]

    ; set the PendSV exception priority
    LDR     r0, =NVIC_SYSPRI2
    LDR     r1, =NVIC_PENDSV_PRI
    LDR.W   r2, [r0,#0x00]       ; read
    ORR     r1, r1, r2           ; modify
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

    ; get thread_switch_interrupt_flag
    LDR     r0, =thread_switch_interrupt_flag
    LDR     r1, [r0]
    CBZ     r1, pendsv_exit         ; pendsv already handled

    ; clear thread_switch_interrupt_flag to 0
    MOV     r1, #0x00
    STR     r1, [r0]

    LDR     r0, =interrupt_from_thread
    LDR     r1, [r0]
    CBZ     r1, switch_to_thread    ; skip register save at the first time

    MRS     r1, psp                 ; get from thread stack pointer
    
#if defined( __ARMVFP__ )
    TST     lr, #0x10
    BNE     skip_push_fpu
    VSTMDB  r1!, {d8 - d15}
skip_push_fpu    
#endif

    STMFD   r1!, {r4 - r11}         ; push r4 - r11 register
    
#if defined( __ARMVFP__ )
    MOV     r4, #0x00
    TST     lr, #0x10
    BNE     push_flag
    MOV     r4, #0x01
push_flag
    SUB     r1, r1, #0x04
    STR     r4, [r1]
#endif

    LDR     r0, [r0]
    STR     r1, [r0]                ; update from thread stack pointer

switch_to_thread
    LDR     r1, =interrupt_to_thread
    LDR     r1, [r1]
    LDR     r1, [r1]                ; load thread stack pointer

#if defined( __ARMVFP__ )
    LDMFD   r1!, {r3}
#endif

    LDMFD   r1!, {r4 - r11}         ; pop r4 - r11 register

#if defined( __ARMVFP__ )
    CBZ     r3,  skip_pop_fpu
    VLDMIA  r1!, {d8 - d15}
skip_pop_fpu    
#endif

    MSR     psp, r1                 ; update stack pointer

pendsv_exit
    ; restore interrupt
    MSR     PRIMASK, r2
    
#if defined( __ARMVFP__ )
    ORR     lr, lr, #0x10
    CBZ     r3, return_without_fpu
    BIC     lr, lr, #0x01
return_without_fpu
#endif

    ORR     lr, lr, #0x04
    BX      lr

;/*
; * void hw_context_switch_to(hz_uint32 to);
; * r0 --> to
; */
    EXPORT hw_context_switch_to
hw_context_switch_to:
    LDR     r1, =interrupt_to_thread
    STR     r0, [r1]
    
#if defined( __ARMVFP__ )
    MRS     r2, CONTROL
    BIC     r2, r2, #0x04
    MSR     CONTROL, r2
#endif    

    ; set from thread to 0
    LDR     r1, =interrupt_from_thread
    MOV     r0, #0x0
    STR     r0, [r1]

    ; set interrupt flag to 1
    LDR     r1, =thread_switch_interrupt_flag
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
    NOP
    NOP

    IMPORT hard_fault_exception
    EXPORT HardFault_Handler
HardFault_Handler:

    ; get current context
    MRS     r0, psp                 ; get fault thread stack pointer
    PUSH    {lr}
    BL      hard_fault_exception
    POP     {lr}

    ORR     lr, lr, #0x04
    BX      lr

    ; never reach here!

; compatible with old version
    EXPORT hw_interrupt_thread_switch
hw_interrupt_thread_switch:
    BX      lr
    
  EXPORT hw_interrupt_disable
hw_interrupt_disable:
        CPSID   I
        BX      LR
        
  EXPORT hw_interrupt_enable
hw_interrupt_enable:
        CPSIE   I
        BX      LR
        
  EXPORT hw_interrupt_suspend          
hw_interrupt_suspend:
        MRS     R0,     PRIMASK
        CPSID   I
        BX      LR
     
  EXPORT hw_interrupt_recover    
hw_interrupt_recover:
        MSR     PRIMASK,   R0
        BX      LR
        
  EXPORT cpu_sleep
cpu_sleep
        WFI
        BX      LR
        
  EXPORT cpu_switch_to_kernel
cpu_switch_to_kernel
        SVC     #32
        NOP
        NOP
        BX      LR
        
  EXPORT SVC_Handler
SVC_Handler
        BIC     LR,       LR,       #0x0F
        ORR     LR,       LR,       #0x01
        BX      LR
        
  
  EXPORT cpu_switch_to_usr
cpu_switch_to_usr
        MRS     R0,       CONTROL
        ORR     R0,       R0,       #0x03
        MSR     CONTROL,  R0
        BX      LR
        
    END
