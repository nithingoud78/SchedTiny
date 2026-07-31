#include "platform_timer.h"

#include "platform_clock.h"

#ifdef STM32
#include "stm32h7xx_hal.h"

/* DWT Registers */
#define DWT_CTRL           (*(volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT         (*(volatile uint32_t *)0xE0001004)
#define DEMCR              (*(volatile uint32_t *)0xE000EDFC)
#define DEMCR_TRCENA       (1 << 24)
#define DWT_CTRL_CYCCNTENA (1 << 0)

static uint32_t system_core_clock = 400000000; /* Default 400 MHz */
#else
#include <time.h>
#endif

void platform_timer_init(void)
{
#ifdef STM32
    system_core_clock = platform_clock_get_core_freq();
    /* Enable TRCENA */
    DEMCR |= DEMCR_TRCENA;
    /* Reset cycle counter */
    DWT_CYCCNT = 0;
    /* Enable cycle counter */
    DWT_CTRL |= DWT_CTRL_CYCCNTENA;
#endif
}

uint32_t platform_get_tick(void)
{
#ifdef STM32
    return HAL_GetTick();
#else
    return (uint32_t)((clock() * 1000) / CLOCKS_PER_SEC);
#endif
}

uint32_t platform_timestamp_us(void)
{
#ifdef STM32
    return DWT_CYCCNT / (system_core_clock / 1000000);
#else
    /* Mock implementation for host */
    return (uint32_t)((clock() * 1000000) / CLOCKS_PER_SEC);
#endif
}

void platform_delay_us(uint32_t us)
{
#ifdef STM32
    uint32_t start = DWT_CYCCNT;
    uint32_t ticks = us * (system_core_clock / 1000000);
    while ((DWT_CYCCNT - start) < ticks)
    {
        /* Busy wait */
    }
#else
    /* Simple busy loop mock for host */
    volatile uint32_t i;
    for (i = 0; i < us * 10; i++)
    {
    }
#endif
}
