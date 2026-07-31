/**
 * @file    drv_uart.c
 * @brief   Implementation of UART DMA abstraction.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "drv_uart.h"

static volatile uint8_t uart_tx_busy = 0;

SchedStatus_t drv_uart_init(uint32_t baudrate)
{
    // Stubbed: Initialize UART3 and DMA1 Stream 3 for Nucleo H743ZI2
    (void)baudrate;
    uart_tx_busy = 0;
    return SCHED_OK;
}

uint8_t drv_uart_is_tx_busy(void)
{
    return uart_tx_busy;
}

SchedStatus_t drv_uart_tx_dma(const uint8_t *data, uint16_t length)
{
    if (uart_tx_busy)
    {
        return SCHED_ERR_STATE;
    }
    if (data == NULL || length == 0)
    {
        return SCHED_ERR_PARAM;
    }

    uart_tx_busy = 1;

    // Stubbed: Set DMA Memory address, set Transfer length, enable DMA Stream,
    // enable UART DMA TX request.

    // Simulating immediate completion for the stub
    uart_tx_busy = 0;

    return SCHED_OK;
}

// In a real implementation, the DMA ISR would look like this:
void DMA1_Stream3_IRQHandler(void)
{
    // Check TCIF (Transfer Complete Interrupt Flag)
    // Clear flag
    // uart_tx_busy = 0;
}
