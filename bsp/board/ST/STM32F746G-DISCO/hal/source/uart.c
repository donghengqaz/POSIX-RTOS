#include "uart.h"

#include "io.h"


void USART1_IRQHandler(void)
{
    OS_RW char data;
    
    data = UART_PRINT->RDR;
}