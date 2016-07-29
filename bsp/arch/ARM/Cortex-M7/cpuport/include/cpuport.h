#ifndef _CPUPORT_H_
#define _CPUPORT_H_

char* thread_stack_init( void entry(void *),
                         void *paramter,
                         char *stack,
                         void *exit );

#endif
