#ifndef PLATFORM_GPIO_H
#define PLATFORM_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        PLATFORM_LED_1 = 0,
        PLATFORM_LED_2,
        PLATFORM_LED_3,
        PLATFORM_LED_MAX
    } platform_led_t;

    /**
     * @brief Initialize all GPIOs used by the demonstration (e.g., LEDs).
     */
    void platform_gpio_init(void);

    /**
     * @brief Write a state to an LED.
     * @param led The LED to write to.
     * @param state true for ON, false for OFF.
     */
    void platform_gpio_write(platform_led_t led, bool state);

    /**
     * @brief Toggle the state of an LED.
     * @param led The LED to toggle.
     */
    void platform_gpio_toggle(platform_led_t led);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_GPIO_H */
