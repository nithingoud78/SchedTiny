#include "platform_gpio.h"

#ifdef STM32
#include "stm32h7xx_hal.h"

/* Nucleo H743ZI2 LED mappings */
#define LED1_PIN  GPIO_PIN_0
#define LED1_PORT GPIOB
#define LED2_PIN  GPIO_PIN_7
#define LED2_PORT GPIOB
#define LED3_PIN  GPIO_PIN_14
#define LED3_PORT GPIOB

static GPIO_TypeDef *get_port(platform_led_t led)
{
    switch (led)
    {
        case PLATFORM_LED_1:
            return LED1_PORT;
        case PLATFORM_LED_2:
            return LED2_PORT;
        case PLATFORM_LED_3:
            return LED3_PORT;
        default:
            return NULL;
    }
}

static uint16_t get_pin(platform_led_t led)
{
    switch (led)
    {
        case PLATFORM_LED_1:
            return LED1_PIN;
        case PLATFORM_LED_2:
            return LED2_PIN;
        case PLATFORM_LED_3:
            return LED3_PIN;
        default:
            return 0;
    }
}
#endif

void platform_gpio_init(void)
{
#ifdef STM32
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin              = LED1_PIN | LED2_PIN | LED3_PIN;
    GPIO_InitStruct.Mode             = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull             = GPIO_NOPULL;
    GPIO_InitStruct.Speed            = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#else
    /* Mock GPIO init */
#endif
}

void platform_gpio_write(platform_led_t led, bool state)
{
#ifdef STM32
    GPIO_TypeDef *port = get_port(led);
    uint16_t pin       = get_pin(led);
    if (port)
    {
        HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
#else
    (void)led;
    (void)state;
#endif
}

void platform_gpio_toggle(platform_led_t led)
{
#ifdef STM32
    GPIO_TypeDef *port = get_port(led);
    uint16_t pin       = get_pin(led);
    if (port)
    {
        HAL_GPIO_TogglePin(port, pin);
    }
#else
    (void)led;
#endif
}
