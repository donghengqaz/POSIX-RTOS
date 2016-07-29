/*
 * File         : cpuport.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-12       DongHeng        create the first version
 */

#include "cpuport.h"
#include "stdio.h"
#include "sched.h"

/*@{*/

/* the stack data structure when interrupt occurss */
struct interrupt_stack_frame
{
    unsigned int r0;
    unsigned int r1;  
    unsigned int r2;
    unsigned int r3;
    unsigned int r12;
    unsigned int lr;
    unsigned int pc;
    unsigned int psr;
};

/* the stack data structure */
struct nomal_stack_frame
{
    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11;  
};

/* the stack data structure */
struct stack_frame
{
#if USING_FPU
    unsigned int flag;
#endif
    struct nomal_stack_frame nomal_stack_frame;
    struct interrupt_stack_frame interrupt_stack_frame;
};

/* the stack data structure with fpu */
struct frame_fpu
{
    unsigned int s0;
    unsigned int s1;
    unsigned int s2;
    unsigned int s3;
    unsigned int s4;
    unsigned int s5;
    unsigned int s6;
    unsigned int s7;
    unsigned int s8;
    unsigned int s9;
    unsigned int s10;
    unsigned int s11;
    unsigned int s12;
    unsigned int s13;
    unsigned int s14;
    unsigned int s15;
    unsigned int fpscr;
    unsigned int reserved;
};

/* the stack data structure with fpu when interrupt occurss */
struct interrupt_stack_frame_fpu
{
    struct interrupt_stack_frame interrupt_stack_frame;
#if USING_FPU
    struct stack_frame_fpu stack_frame_fpu;
#endif
};

/* the stack data structure with fpu */
struct stack_frame_fpu
{
    unsigned int flag;

    struct nomal_stack_frame nomal_stack_frame;    
  
    struct frame_fpu frame_fpu;
    
    struct interrupt_stack_frame interrupt_stack_frame;
};

/*@}*/


/*@{*/

char* pthread_hw_stack_init(void entry(void *), 
                            void *paramter, 
                            char *stack, 
                            void *exit(void *))
{
    struct stack_frame *stack_frame;
    char *stk;
    int i;
  
    stk = stack + sizeof(unsigned int);
    stk = (char *)ALIGN((unsigned int)stk);
    stk = stk - sizeof(struct stack_frame);
    
    stack_frame = (struct stack_frame *)stk;
    
    for( i = 0; i < sizeof(struct stack_frame) / sizeof( int ); i++ )
    {  
        ((unsigned int *)stack_frame)[i] = 0xdeadbeef;
    }
    
    stack_frame->interrupt_stack_frame.r0  = (unsigned long)paramter;
    stack_frame->interrupt_stack_frame.r1  = 0;
    stack_frame->interrupt_stack_frame.r2  = 0;
    stack_frame->interrupt_stack_frame.r3  = 0;
    stack_frame->interrupt_stack_frame.r12 = 0;
    stack_frame->interrupt_stack_frame.lr  = (int)exit;
    stack_frame->interrupt_stack_frame.pc  = (int)entry;
    stack_frame->interrupt_stack_frame.psr = 0x01000000L;
   
#if USING_FPU    
    stack_frame->flag = 0;
#endif
    
    return stk;
}

void hard_fault_exception(struct interrupt_stack_frame *interrupt_stack_frame)
{	
    printk("R0  is 0x%08x .\r\n", interrupt_stack_frame->r0);
    printk("R1  is 0x%08x .\r\n", interrupt_stack_frame->r1);
    printk("R2  is 0x%08x .\r\n", interrupt_stack_frame->r2);
    printk("R3  is 0x%08x .\r\n", interrupt_stack_frame->r3);
	
    printk("R12 is 0x%08x .\r\n", interrupt_stack_frame->r12);
    printk("LR  is 0x%08x .\r\n", interrupt_stack_frame->lr);
    printk("PC  is 0x%08x .\r\n", interrupt_stack_frame->pc);
    printk("PSR is 0x%08x .\r\n", interrupt_stack_frame->pc); 
    
    while(1);
}


/*@}*/
