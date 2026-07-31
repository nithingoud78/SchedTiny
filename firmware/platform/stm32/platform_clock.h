#ifndef PLATFORM_CLOCK_H
#define PLATFORM_CLOCK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize the system clock (e.g., PLL configuration).
     */
    void platform_clock_init(void);

    /**
     * @brief Get the configured system core clock frequency in Hz.
     * @return Core clock frequency in Hz.
     */
    uint32_t platform_clock_get_core_freq(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_CLOCK_H */
