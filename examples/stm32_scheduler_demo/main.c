/**
 * @file main.c
 * @brief STM32 Hardware Demonstration for SchedTiny
 *
 * Runs HPF, EDF, and RMS scheduling policies on actual hardware.
 * Evaluates performance and exports JSON/CSV over standard output.
 */

#include "sched_benchmark.h"
#include "sched_core.h"

#include <stdio.h>
#include <string.h>

#include "platform_gpio.h"
#include "platform_init.h"
#include "platform_timer.h"

/* Workload task implementations */
static void demo_task_1(void *arg)
{
    (void)arg;
    platform_gpio_toggle(PLATFORM_LED_1);
    platform_delay_us(500); /* 500 us execution */
}

static void demo_task_2(void *arg)
{
    (void)arg;
    platform_gpio_toggle(PLATFORM_LED_2);
    platform_delay_us(1000); /* 1000 us execution */
}

static void demo_task_3(void *arg)
{
    (void)arg;
    platform_gpio_toggle(PLATFORM_LED_3);
    platform_delay_us(2000); /* 2000 us execution */
}

static void run_demo_benchmark(void)
{
    sched_benchmark_ctx_t ctx;
    sched_benchmark_init(&ctx);

    /* Configure workload (Small profile equivalent) */
    sched_benchmark_config_t cfg = {.seed           = 42,
                                    .duration_ticks = 10000,
                                    .task_count     = 3,
                                    .tasks          = {
                                        {.id        = 1,
                                         .priority  = 3,
                                         .period    = 100,
                                         .deadline  = 100,
                                         .exec_time = 5,
                                         .func      = demo_task_1},
                                        {.id        = 2,
                                         .priority  = 2,
                                         .period    = 200,
                                         .deadline  = 200,
                                         .exec_time = 10,
                                         .func      = demo_task_2},
                                        {.id        = 3,
                                         .priority  = 1,
                                         .period    = 400,
                                         .deadline  = 400,
                                         .exec_time = 20,
                                         .func      = demo_task_3},
                                    }};

    printf("Starting SchedTiny Hardware Demonstration...\n");

    /* Run HPF */
    printf("Running HPF...\n");
    sched_benchmark_run(&ctx, SCHED_POLICY_HPF, &cfg);

    /* Run EDF */
    printf("Running EDF...\n");
    sched_benchmark_run(&ctx, SCHED_POLICY_EDF, &cfg);

    /* Run RMS */
    printf("Running RMS...\n");
    sched_benchmark_run(&ctx, SCHED_POLICY_RMS, &cfg);

    /* Export Results */
    char buffer[1024];

    printf("\n--- CSV EXPORT ---\n");
    sched_benchmark_export_csv(&ctx, buffer, sizeof(buffer));
    printf("%s", buffer);

    printf("\n--- JSON EXPORT ---\n");
    sched_benchmark_export_json(&ctx, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}

int main(void)
{
    /* Initialize STM32 platform (HAL, clocks, GPIO, DWT) */
    platform_init();

    /* Toggle LEDs to indicate boot */
    platform_gpio_write(PLATFORM_LED_1, true);
    platform_gpio_write(PLATFORM_LED_2, true);
    platform_gpio_write(PLATFORM_LED_3, true);
    platform_delay_us(500000);
    platform_gpio_write(PLATFORM_LED_1, false);
    platform_gpio_write(PLATFORM_LED_2, false);
    platform_gpio_write(PLATFORM_LED_3, false);

    /* Run benchmark suite */
    run_demo_benchmark();

    /* Infinite loop */
    while (1)
    {
        platform_gpio_toggle(PLATFORM_LED_1);
        platform_delay_us(250000);
    }
    return 0;
}
