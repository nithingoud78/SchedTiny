// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on

#include "sched_mc.h"

static void test_mc_init(void **state)
{
    (void)state;
    sched_mc_t ctx;
    assert_int_equal(sched_mc_init(NULL), SCHED_ERR_PARAM);
    assert_int_equal(sched_mc_init(&ctx), SCHED_OK);
    assert_int_equal(ctx.mode, SCHED_MC_MODE_LO);
    assert_true(ctx.initialized);
}

static void test_mc_register(void **state)
{
    (void)state;
    sched_mc_t ctx;
    sched_mc_init(&ctx);

    /* Register a LO task */
    assert_int_equal(sched_mc_register(&ctx, 1, SCHED_MC_CRIT_LO, 10, 10, 5, 100), SCHED_OK);
    /* Register a HI task */
    assert_int_equal(sched_mc_register(&ctx, 2, SCHED_MC_CRIT_HI, 20, 30, 10, 200), SCHED_OK);

    /* Duplicate task */
    assert_int_equal(sched_mc_register(&ctx, 1, SCHED_MC_CRIT_LO, 5, 5, 2, 50), SCHED_ERR_PARAM);
}

static void test_mc_dispatch_lo_mode(void **state)
{
    (void)state;
    sched_mc_t ctx;
    sched_mc_init(&ctx);

    /* Priority: higher number = higher priority */
    sched_mc_register(&ctx, 1, SCHED_MC_CRIT_LO, 10, 10, 5, 100);
    sched_mc_register(&ctx, 2, SCHED_MC_CRIT_HI, 20, 30, 10, 200);

    uint32_t task_id = SCHED_MC_NO_TASK;
    assert_int_equal(sched_mc_dispatch(&ctx, &task_id), SCHED_OK);
    assert_int_equal(task_id, 2); /* Task 2 has priority 10 */

    /* Remove task 2 */
    sched_mc_remove(&ctx, 2);
    assert_int_equal(sched_mc_dispatch(&ctx, &task_id), SCHED_OK);
    assert_int_equal(task_id, 1); /* Task 1 is next */
}

static void test_mc_mode_switch(void **state)
{
    (void)state;
    sched_mc_t ctx;
    sched_mc_init(&ctx);

    sched_mc_register(&ctx, 1, SCHED_MC_CRIT_LO, 10, 10, 5, 100);
    sched_mc_register(&ctx, 2, SCHED_MC_CRIT_HI, 2, 10, 10, 200);

    /* Initialize budgets */
    sched_mc_reset_budget(&ctx, 1);
    sched_mc_reset_budget(&ctx, 2);

    /* Simulate tick for task 2 */
    assert_int_equal(sched_mc_tick(&ctx, 2, 0), SCHED_OK);  // budget = 1
    assert_int_equal(ctx.mode, SCHED_MC_MODE_LO);

    /* Simulate tick for task 2 causing it to overrun LO WCET (2) */
    assert_int_equal(sched_mc_tick(&ctx, 2, 1), SCHED_OK);  // budget = 0
    assert_int_equal(ctx.mode, SCHED_MC_MODE_HI);           /* Should switch to HI mode */
    assert_int_equal(ctx.mode_switch_count, 1);

    /* In HI mode, task 1 (LO) should be dropped */
    uint32_t task_id = SCHED_MC_NO_TASK;
    /* Task 2 completed its LO budget, but wait, if it completed it, what is its
     * budget now? */
    /* When mode switches to HI, reset_budget must be called explicitly for the HI
     * task, but in our static benchmark, reset_budget is called on release. If
     * the dispatch only selects active tasks, wait, if current_budget is 0, it
     * doesn't run? No, sched_mc_dispatch doesn't check current_budget. It just
     * selects active and not dropped tasks. */
    assert_int_equal(sched_mc_dispatch(&ctx, &task_id), SCHED_OK);
    assert_int_equal(task_id, 2);  // Task 2 is HI, still active

    /* Test idle recovery */
    /* Tick with no task to simulate idle */
    assert_int_equal(sched_mc_tick(&ctx, SCHED_MC_NO_TASK, 2), SCHED_OK);
    assert_int_equal(ctx.mode, SCHED_MC_MODE_LO);
    assert_int_equal(ctx.mode_switch_count, 2);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_mc_init),
        cmocka_unit_test(test_mc_register),
        cmocka_unit_test(test_mc_dispatch_lo_mode),
        cmocka_unit_test(test_mc_mode_switch),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
