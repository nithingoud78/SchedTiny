#ifndef PLATFORM_TIMER_H
#define PLATFORM_TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize the hardware timer (e.g., DWT cycle counter) for microsecond precision.
     */
    void platform_timer_init(void);

    /**
     * @brief Get the current system tick count in milliseconds.
     * @return Milliseconds since boot.
     */
    uint32_t platform_get_tick(void);

    /**
     * @brief Get the current timestamp in microseconds using the DWT cycle counter.
     * @return Microseconds since boot.
     */
    uint32_t platform_timestamp_us(void);

    /**
     * @brief Blocking delay for a specified number of microseconds.
     * @param us Number of microseconds to delay.
     */
    void platform_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_TIMER_H */
