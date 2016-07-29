#include "low_level.h"
#include "io.h"

#include "stdio.h"

static void uart_com_clk_init(void)
{
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __USART1_CLK_ENABLE();
}

static void uart_com_init(void)
{
    UART_HandleTypeDef UartHandle;

    UartHandle.Init.BaudRate   = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;
    
    UartHandle.Instance        = UART_PRINT;
    
    UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;  
    
    if (HAL_UART_DeInit(&UartHandle) != HAL_OK)
        while(1);

    if (HAL_UART_Init(&UartHandle) != HAL_OK)
        while(1);
    
    //__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_RXNE);
}

static void uart_com_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    
    GPIO_InitStruct.Pin       = UART_TX_GPIO_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = UART_TX_AF;

    HAL_GPIO_Init(UART_TX_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART_RX_GPIO_PIN;
    GPIO_InitStruct.Alternate = UART_RX_AF;

    HAL_GPIO_Init(UART_RX_GPIO_PORT, &GPIO_InitStruct);    
}

static void uart_com_nvic_init(void)
{
    HAL_NVIC_SetPriority(UART_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(UART_IRQn);
}

void uart_configuration(void)
{
    uart_com_clk_init();
    uart_com_gpio_init();
    uart_com_nvic_init();
    uart_com_init();
}

size_t low_level_init(void)
{
    uart_configuration();
    
    printk("low level init ok.\r\n");
  
    return (0);
}

int fputc(char ch)
{
    while (!(UART_PRINT->ISR & UART_FLAG_TC)){}
      
    UART_PRINT->TDR = (char)ch; 
  
    return (ch);
}