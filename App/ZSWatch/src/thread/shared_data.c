#include "shared_data.h"
#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(shared_data, LOG_LEVEL_INF);

/* ════════════════════════════════════════════════════════════════
 *  Global shared state
 * ════════════════════════════════════════════════════════════════ */
struct watch_raw_data       g_raw;
struct watch_processed_data g_processed;

/* ════════════════════════════════════════════════════════════════
 *  Semaphores
 *  Initial count = 0, max count = 1 (binary semaphore pattern).
 * ════════════════════════════════════════════════════════════════ */
K_SEM_DEFINE(sem_sample_tick,  0, 1);
K_SEM_DEFINE(sem_sensor_ready, 0, 1);
K_SEM_DEFINE(sem_ble_notify,   0, 1);
K_SEM_DEFINE(sem_sleep,        0, 1);

/* ════════════════════════════════════════════════════════════════
 *  Mutex
 * ════════════════════════════════════════════════════════════════ */
K_MUTEX_DEFINE(mutex_data);

/* ════════════════════════════════════════════════════════════════
 *  Timers
 *
 *  timer0 – 30 s periodic → sem_sleep  (sleep / low-power trigger)
 *  timer1 – 100 ms periodic → sem_sample_tick (sensor sample tick)
 * ════════════════════════════════════════════════════════════════ */
static void timer0_handler(struct k_timer *t)
{
    ARG_UNUSED(t);
    k_sem_give(&sem_sleep);
}

static void timer1_handler(struct k_timer *t)
{
    ARG_UNUSED(t);
    k_sem_give(&sem_sample_tick);
}

K_TIMER_DEFINE(timer0, timer0_handler, NULL);  /* sleep  – 30 s  */
K_TIMER_DEFINE(timer1, timer1_handler, NULL);  /* sample – 100 ms */

/* ════════════════════════════════════════════════════════════════
 *  API
 * ════════════════════════════════════════════════════════════════ */
void shared_data_init(void)
{
    memset(&g_raw,       0, sizeof(g_raw));
    memset(&g_processed, 0, sizeof(g_processed));
    LOG_INF("Shared data initialised");
}

void shared_data_timers_start(void)
{
    k_timer_start(&timer0, K_SECONDS(30), K_SECONDS(30));
    k_timer_start(&timer1, K_MSEC(100),   K_MSEC(100));
    LOG_INF("Timers started: timer0=30s, timer1=100ms");
}

void shared_data_sleep_timer_reset(void)
{
    k_timer_start(&timer0, K_SECONDS(30), K_SECONDS(30));
}
