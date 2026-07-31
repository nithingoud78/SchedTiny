/**
 * @file    mock_hal.c
 * @brief   Mock hardware abstraction layer for host-side unit testing.
 */

#include "drv_gpio.h"
#include "drv_timer.h"
#include "drv_uart.h"
// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on
#include <string.h>

// Mock variables
uint32_t mock_dwt_counter = 0;
uint8_t mock_uart_busy    = 0;
char mock_uart_buffer[4096];
uint16_t mock_uart_len = 0;

// DRV TIMER MOCKS
void drv_timer_dwt_init(void)
{
    mock_dwt_counter = 0;
}

uint32_t drv_timer_dwt_now(void)
{
    return mock_dwt_counter;
}

void drv_timer_isr_gen_init(uint8_t timer_id, uint32_t rate_hz)
{
}

void drv_timer_delay_us(uint32_t us)
{
}

// DRV UART MOCKS
SchedStatus_t drv_uart_init(uint32_t baudrate)
{
    return SCHED_OK;
}

uint8_t drv_uart_is_tx_busy(void)
{
    return mock_uart_busy;
}

SchedStatus_t drv_uart_tx_dma(const uint8_t *data, uint16_t length)
{
    if (mock_uart_busy)
    {
        return SCHED_ERR_STATE;
    }
    if (length > sizeof(mock_uart_buffer))
    {
        length = sizeof(mock_uart_buffer);
    }
    memcpy(mock_uart_buffer, data, length);
    mock_uart_buffer[length] = '\0';  // Null terminate for easy string comparison in tests
    mock_uart_len            = length;
    mock_uart_busy           = 1;
    return SCHED_OK;
}

// DRV GPIO MOCKS
SchedStatus_t drv_gpio_init(void)
{
    return SCHED_OK;
}

void drv_gpio_set(DrvGpioPin_t pin)
{
}

void drv_gpio_reset(DrvGpioPin_t pin)
{
}

void drv_gpio_toggle(DrvGpioPin_t pin)
{
}
