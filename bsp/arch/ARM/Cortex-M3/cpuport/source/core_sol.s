
    SECTION .sol:CODE:NOROOT(2)
    THUMB
    REQUIRE8
    PRESERVE8
	
    IMPORT hard_track_report
  EXPORT HardFault_Handler
HardFault_Handler:
        MRS     R0, 	msp
        STMFD   R0!,	{R4-R11}
        MSR     msp,	r0
        PUSH	{lr}
        BL      hard_track_report
        POP     {lr}
bbp
        MOV     R0,	R1
        BL      bbp
        BX      LR
        
  EXPORT read_core_ctrl
read_core_ctrl:
        MRS	R0,     CONTROL
        BX      LR

  EXPORT hz_hw_interrupt_disable
hz_hw_interrupt_disable:
        CPSID   I
        BX      LR
        
  EXPORT hz_hw_interrupt_enable
hz_hw_interrupt_enable:
        CPSIE   I
        BX      LR

  EXPORT hz_hw_interrupt_suspend          
hz_hw_interrupt_suspend:
        MRS     R0,     PRIMASK
        CPSID   I
        BX      LR
        
  EXPORT hz_hw_interrupt_recover    
hz_hw_interrupt_recover:
        MSR     PRIMASK,   R0
        BX      LR

  EXPORT cpu_sleep
cpu_sleep
        WFI
        BX      LR
        
  EXPORT system_boot
system_boot
        LDR     R4,     [R0]
        msr     msp,    R4
        ADD     R0,     R0,     #4
        LDR     R4,     [R0]
        BX      R4
        
	END
        
	
	
