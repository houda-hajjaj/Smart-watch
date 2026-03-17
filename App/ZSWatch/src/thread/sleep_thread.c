#include "sleep_thread.h"
#include "shared_data.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sleep_thread, LOG_LEVEL_INF);

/* ── Thread parameters ─────────────────────────────────────────── */
#define SLEEP_STACK_SIZE   512
#define SLEEP_PRIORITY     3   /* higher priority than other app threads */

/* ── Internal state ────────────────────────────────────────────── */
static volatile bool g_sleeping;

/* ════════════════════════════════════════════════════════════════
 *  Thread function
 *
 *  Flow:
 *    wait(sem_sleep)                ← fired every 30 s by timer0
 *    → suspend sensor + processing + ble threads (low-power)
 *    → set sleep flag
 *    → wait for motion / wake event (sem_sample_tick as proxy)
 *    → resume all threads
 *    → reset sleep timer
 * ════════════════════════════════════════════════════════════════ */

/* Thread IDs defined via K_THREAD_DEFINE in their own .c files */
extern const k_tid_t sensor_tid;
extern const k_tid_t processing_tid;
extern const k_tid_t ble_tid;

static void sleep_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        /* Block until 30-second idle timer fires */
        k_sem_take(&sem_sleep, K_FOREVER);

        LOG_INF("Entering sleep mode");
        g_sleeping = true;

        /* Suspend data-pipeline threads to save power.
         * In a full implementation replace with pm_state_force()
         * to engage the SoC low-power state. */
        k_thread_suspend(sensor_tid);
        k_thread_suspend(processing_tid);
        k_thread_suspend(ble_tid);

        /* Wait here until the sample tick fires again  –
         * this happens when the user moves the watch and
         * shared_data_sleep_timer_reset() restarts timer1.
         * We use a fresh sem_take with a long timeout as wake check. */
        k_sem_take(&sem_sample_tick, K_FOREVER);

        /* Resume the pipeline */
        k_thread_resume(sensor_tid);
        k_thread_resume(processing_tid);
        k_thread_resume(ble_tid);

        g_sleeping = false;
        LOG_INF("Woke from sleep");

        /* Restart the idle timer */
        shared_data_sleep_timer_reset();
    }
}

K_THREAD_DEFINE(sleep_tid, SLEEP_STACK_SIZE,
                sleep_thread_fn, NULL, NULL, NULL,
                SLEEP_PRIORITY, 0, 0);

/* ════════════════════════════════════════════════════════════════
 *  Public API
 * ════════════════════════════════════════════════════════════════ */
bool sleep_is_active(void)
{
    return g_sleeping;
}
