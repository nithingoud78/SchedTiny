#include "platform_init.h"

#include "platform_clock.h"
#include "platform_gpio.h"
#include "platform_timer.h"

#ifdef STM32
#include "stm32h7xx_hal.h"
#else
#include <stdio.h>
#endif

void platform_init(void)
{
#ifdef STM32
    HAL_Init();
    platform_clock_init();
    platform_gpio_init();
    platform_timer_init();
#else
    /* Host environment initialization (no-op) */
    platform_clock_init();
    platform_gpio_init();
    platform_timer_init();
#endif
}

void platform_tick(void)
{
#ifdef STM32
    HAL_IncTick();
#else
    /* Host environment tick (handled by simulation) */
#endif
}

#ifdef STM32
/* Re-target printf to ITM (Data Watchpoint and Trace) or UART */
int _write(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++)
    {
        ITM_SendChar(*ptr++);
    }
    return len;
}
#endif
