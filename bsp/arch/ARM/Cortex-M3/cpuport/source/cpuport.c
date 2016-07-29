#include "cpuport.h"

#include "io.h"
#include "stdio.h"

typedef struct
{
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;
}CORE_REG_LOW;

typedef struct
{
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
}CORE_REG_HIGT;

typedef struct
{
	uint32_t core_return;
	CORE_REG_HIGT reg_high;
	CORE_REG_LOW reg_low;
}CORE_REG;

typedef struct
{  
    CORE_REG_HIGT       Core_RegH; 
    CORE_REG_LOW        Core_RegL;
}STACK_FRAME;

void hard_track_report(CORE_REG *core_reg)
{
  extern void sched_status_report(void);
  
  printk("\r\nHard default in %s mode.\r\n", (core_reg->core_return & 0x00000004) ? ("process") : ("irq"));
	
  printk("R0  is 0x%08x .\r\n", core_reg->reg_low.r0);
  printk("R1  is 0x%08x .\r\n", core_reg->reg_low.r1);
  printk("R2  is 0x%08x .\r\n", core_reg->reg_low.r2);
  printk("R3  is 0x%08x .\r\n", core_reg->reg_low.r3);
	
  printk("R4  is 0x%08x .\r\n", core_reg->reg_high.r4);
  printk("R5  is 0x%08x .\r\n", core_reg->reg_high.r5);
  printk("R6  is 0x%08x .\r\n", core_reg->reg_high.r6);
  printk("R7  is 0x%08x .\r\n", core_reg->reg_high.r7);
  printk("R8  is 0x%08x .\r\n", core_reg->reg_high.r8);
  printk("R9  is 0x%08x .\r\n", core_reg->reg_high.r9);
  printk("R10 is 0x%08x .\r\n", core_reg->reg_high.r10);
  printk("R11 is 0x%08x .\r\n", core_reg->reg_high.r11);
	
  printk("R12 is 0x%08x .\r\n", core_reg->reg_low.r12);
  printk("LR  is 0x%08x .\r\n", core_reg->reg_low.lr);
  printk("PC  is 0x%08x .\r\n", core_reg->reg_low.pc);
  printk("PSR is 0x%08x .\r\n", core_reg->reg_low.psr);

  sched_status_report();

  while(1);
}

char* pthread_hw_stack_init(void entry(void *), void *paramter, char *stack, void *exit)
{
    STACK_FRAME *Stack_Frame;
    char *stk;
    int i;
  
    stk = stack + sizeof( int );
    stk = stk - sizeof( STACK_FRAME );
    
    Stack_Frame = (STACK_FRAME *)stk;
    
    for( i = 0; i < sizeof( STACK_FRAME ) / sizeof( int ); i++ )
    {  
        ((int*)Stack_Frame)[i] = 0xdeadbeef;
    }
    
    Stack_Frame->Core_RegL.r0  = (unsigned long)paramter;
    Stack_Frame->Core_RegL.r1  = 0;
    Stack_Frame->Core_RegL.r2  = 0;
    Stack_Frame->Core_RegL.r3  = 0;
    Stack_Frame->Core_RegL.r12 = 0;
    Stack_Frame->Core_RegL.lr  = (int)exit;
    Stack_Frame->Core_RegL.pc  = (int)entry;
    Stack_Frame->Core_RegL.psr = 0x01000000L;
    
    return stk;
}
