#include "low_level.h"

#include "io.h"

#include "stdio.h"

static void uart_clk_init(void)
{
    RCC_APB2PeriphClockCmd(USART_PRINT_RX_CLK | USART_PRINT_TX_CLK | RCC_APB2Periph_AFIO, ENABLE);
#if (USART_PRINT_NUM == 1)
    RCC_APB2PeriphClockCmd(USART_PRINT_CLK, ENABLE);
#else
    RCC_APB1PeriphClockCmd(USART_PRINT_CLK, ENABLE);
#endif
}

static void uart_gpio_conf(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = USART_PRINT_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USART_PRINT_TX_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin   = USART_PRINT_RX_PIN;
    GPIO_Init(USART_PRINT_RX_PORT, &GPIO_InitStructure);
}

static void uart_nvic_conf(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART_PRINT_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void uart_init(void)
{
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART_PRINT, &USART_InitStructure);
    
    USART_ITConfig(USART_PRINT, USART_IT_RXNE, DISABLE);
    
    USART_Cmd(USART_PRINT, ENABLE);
}

err_t low_level_init(void)
{
    uart_clk_init();
    uart_gpio_conf();
    uart_nvic_conf();
    uart_init();
    
    printk("low level init.\r\n");
    
    return 0;
}

size_t low_level_ram_end(void)
{
    return (0x2000ffff);
}

#define PUTCHAR_PROTOTYPE int fputc(char ch)

PUTCHAR_PROTOTYPE
{
  while (USART_GetFlagStatus(USART_PRINT, USART_FLAG_TC) == RESET){
    ;
  }
  
  USART_SendData( USART_PRINT, ch );

  return ch;
}
