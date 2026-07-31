#include "platform_clock.h"

#ifdef STM32
#include "stm32h7xx_hal.h"
#endif

void platform_clock_init(void)
{
#ifdef STM32
    /* Typically done by STM32Cube generated SystemClock_Config() */
    /* This is just a stub for the demo application context */
#endif
}

uint32_t platform_clock_get_core_freq(void)
{
#ifdef STM32
    return HAL_RCC_GetSysClockFreq();
#else
    return 400000000; /* Mock 400 MHz */
#endif
}
