/**
 * @file    board_config.h
 * @brief   Board-specific configuration for STM32 NUCLEO-H743ZI2.
 *
 * All board-specific constants, pin assignments, and peripheral mappings
 * for the primary SchedTiny target. This file is included ONLY by HAL
 * source files. Do NOT include this directly in scheduler/ or tasks/.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 */

#ifndef SCHEDTINY_BOARD_CONFIG_H
#define SCHEDTINY_BOARD_CONFIG_H

/* =========================================================================
 * Clock
 * ========================================================================= */
#define BOARD_SYSCLK_HZ       480000000UL   /**< Cortex-M7 @ 480 MHz */
#define BOARD_CYCLES_PER_US   (BOARD_SYSCLK_HZ / 1000000UL)  /**< 480 cycles/µs */

/* =========================================================================
 * UART (ST-LINK VCP via USART3)
 * ========================================================================= */
#define BOARD_UART_INSTANCE   USART3
#define BOARD_UART_BAUD       115200UL
#define BOARD_UART_TX_PIN     GPIO_PIN_8    /**< PD8 */
#define BOARD_UART_TX_PORT    GPIOD
#define BOARD_UART_RX_PIN     GPIO_PIN_9    /**< PD9 */
#define BOARD_UART_RX_PORT    GPIOD
#define BOARD_UART_AF         GPIO_AF7_USART3

/* =========================================================================
 * I2C Buses
 * ========================================================================= */
/** I²C1 — MPU6050 Sensor */
#define BOARD_I2C1_SCL_PIN    GPIO_PIN_8    /**< PB8 */
#define BOARD_I2C1_SCL_PORT   GPIOB
#define BOARD_I2C1_SDA_PIN    GPIO_PIN_9    /**< PB9 */
#define BOARD_I2C1_SDA_PORT   GPIOB

/** I²C2 — INA219 Power Monitor */
#define BOARD_I2C2_SCL_PIN    GPIO_PIN_0    /**< PF0 */
#define BOARD_I2C2_SCL_PORT   GPIOF
#define BOARD_I2C2_SDA_PIN    GPIO_PIN_1    /**< PF1 */
#define BOARD_I2C2_SDA_PORT   GPIOF

/** MPU6050 I²C address (AD0 = GND -> 0x68) */
#define BOARD_MPU6050_I2C_ADDR  (0x68U << 1)   /**< Left-shifted for HAL */

/** INA219 I²C address (A0=GND, A1=GND -> 0x40) */
#define BOARD_INA219_I2C_ADDR   (0x40U << 1)

/* =========================================================================
 * Logic Analyzer GPIO Trigger Pins
 * ========================================================================= */
#define BOARD_GPIO_TRIG_PID_START_PIN    GPIO_PIN_0    /**< PC0 */
#define BOARD_GPIO_TRIG_PID_START_PORT   GPIOC
#define BOARD_GPIO_TRIG_PID_END_PIN      GPIO_PIN_1    /**< PC1 */
#define BOARD_GPIO_TRIG_PID_END_PORT     GPIOC
#define BOARD_GPIO_TRIG_INFER_START_PIN  GPIO_PIN_2    /**< PC2 */
#define BOARD_GPIO_TRIG_INFER_START_PORT GPIOC
#define BOARD_GPIO_TRIG_INFER_END_PIN    GPIO_PIN_3    /**< PC3 */
#define BOARD_GPIO_TRIG_INFER_END_PORT   GPIOC
#define BOARD_GPIO_TRIG_ISR_PIN          GPIO_PIN_2    /**< PG2 */
#define BOARD_GPIO_TRIG_ISR_PORT         GPIOG

/* =========================================================================
 * ISR Load Generator Timers
 * ========================================================================= */
#define BOARD_ISRGEN_TIM0     TIM2           /**< Primary ISR load timer */
#define BOARD_ISRGEN_TIM1     TIM3           /**< Backup ISR load timer */
#define BOARD_ISRGEN_TIM_CLK  (BOARD_SYSCLK_HZ / 4UL) /**< APB1 timer clock */

/* =========================================================================
 * Memory Layout
 * ========================================================================= */
#define BOARD_FLASH_SIZE_BYTES  (2UL * 1024UL * 1024UL)   /**< 2 MB Flash */
#define BOARD_SRAM_SIZE_BYTES   (1UL * 1024UL * 1024UL)   /**< 1 MB DTCM+AXI SRAM */

/** TFLM tensor arena size (tuned for KWS DS-CNN on H743ZI2) */
#define BOARD_TFLM_ARENA_SIZE_BYTES   (256U * 1024U)

#endif /* SCHEDTINY_BOARD_CONFIG_H */
