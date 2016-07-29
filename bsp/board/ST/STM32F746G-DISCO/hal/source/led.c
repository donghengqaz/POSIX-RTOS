#include "led.h"
#include "io.h"
#include "led_trigger.h"

static void led_init(void)
{
    GPIO_InitTypeDef  gpio_init_structure;
  
    __HAL_RCC_GPIOI_CLK_ENABLE();
   
    gpio_init_structure.Pin   = GPIO_PIN_1;
    gpio_init_structure.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull  = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_HIGH; 
    HAL_GPIO_Init(GPIOI, &gpio_init_structure);
    
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);
}

static void led_on(void)
{
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_SET);    
}

static void led_off(void)
{
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);
}

err_t led_configuration(void)
{
    struct led_heart_trigger led_heart_trigger;
    
    led_init();
  
    led_heart_trigger.led_count = 0;
    led_heart_trigger.led_off = led_off;
    led_heart_trigger.led_on = led_on;
    
    return led_heart_trigger_init(&led_heart_trigger);
}