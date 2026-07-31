/**
 * @file    drv_uart.h
 * @brief   Hardware Abstraction Layer — Non-blocking UART with DMA.
 *
 * Provides an interface to transmit strings over UART without CPU blocking.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_DRV_UART_H
#define SCHEDTINY_DRV_UART_H

#include "schedtiny_status.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief   Initialize the UART peripheral and its DMA channel.
     * @param[in] baudrate Target baudrate (e.g., 115200).
     * @return  SCHED_OK on success.
     */
    SchedStatus_t drv_uart_init(uint32_t baudrate);

    /**
     * @brief   Check if a UART DMA transmission is currently active.
     * @return  1 if active (busy), 0 if idle.
     */
    uint8_t drv_uart_is_tx_busy(void);

    /**
     * @brief   Start a non-blocking DMA transmission of data over UART.
     * @param[in] data   Pointer to the buffer to transmit. Must remain valid during transmission.
     * @param[in] length Number of bytes to transmit.
     * @return  SCHED_OK if started, SCHED_ERR_STATE if already busy.
     */
    SchedStatus_t drv_uart_tx_dma(const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_DRV_UART_H */
