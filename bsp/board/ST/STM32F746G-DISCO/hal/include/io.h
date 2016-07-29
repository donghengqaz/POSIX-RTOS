#ifndef _IO_H_
#define _IO_H_

#include "rtos.h"

#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#ifdef STLINK_VCP_COM
    #define UART_PRINT                  USART1
    #define UART_IRQn                   USART1_IRQn

    #define UART_TX_GPIO_PIN            GPIO_PIN_9
    #define UART_TX_GPIO_PORT           GPIOA
    #define UART_TX_AF                  GPIO_AF7_USART1

    #define UART_RX_GPIO_PIN            GPIO_PIN_7
    #define UART_RX_GPIO_PORT           GPIOB
    #define UART_RX_AF                  GPIO_AF7_USART1
#endif

#endif
