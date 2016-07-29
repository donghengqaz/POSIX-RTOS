#ifndef _IO_H_
#define _IO_H_

#include "stm32f10x.h"

#define USART_PRINT_NUM                             1

#if( USART_PRINT_NUM == 1 )
    #define USART_PRINT 							USART1
    #define USART_PRINT_IRQ							USART1_IRQn
    #define USART_PRINT_CLK							RCC_APB2Periph_USART1
    #define USART_PRINT_TX_CLK						RCC_APB2Periph_GPIOA
    #define USART_PRINT_TX_PORT						GPIOA
    #define USART_PRINT_TX_PIN						GPIO_Pin_9
    #define USART_PRINT_RX_CLK 						RCC_APB2Periph_GPIOA
    #define USART_PRINT_RX_PORT						GPIOA
    #define USART_PRINT_RX_PIN						GPIO_Pin_10
#elif( USART_PRINT_NUM == 5 )
    #define USART_PRINT 							UART5
    #define USART_PRINT_IRQ							UART5_IRQn
    #define USART_PRINT_CLK							RCC_APB1Periph_UART5
    #define USART_PRINT_TX_CLK						RCC_APB2Periph_GPIOC
    #define USART_PRINT_TX_PORT						GPIOC
    #define USART_PRINT_TX_PIN						GPIO_Pin_12
    #define USART_PRINT_RX_CLK 						RCC_APB2Periph_GPIOD
    #define USART_PRINT_RX_PORT						GPIOD
    #define USART_PRINT_RX_PIN						GPIO_Pin_2
#endif

#endif
