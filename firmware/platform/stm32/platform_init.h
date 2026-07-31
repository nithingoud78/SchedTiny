#ifndef PLATFORM_INIT_H
#define PLATFORM_INIT_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize the hardware platform (clock, peripherals, HAL).
     *        Hides all STM32-specific initialization from the scheduler.
     */
    void platform_init(void);

    /**
     * @brief SysTick or base timer interrupt handler.
     *        To be called by the platform's tick ISR.
     */
    void platform_tick(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_INIT_H */
