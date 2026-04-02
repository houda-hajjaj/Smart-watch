#include "thread/power_thread.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(power_thread, LOG_LEVEL_INF);

#define POWER_TIMEOUT_MS 15000 // 15s
K_SEM_DEFINE(power_awake_sem, 0, 1);

static bool power_thread_started;
static atomic_t eco_mode;
static atomic_t last_activity_ms;

void power_thread_start(void)
{
    if (power_thread_started) {
        return;
    }

    power_thread_started = true;
    (void)atomic_set(&eco_mode, 0);
    (void)atomic_set(&last_activity_ms, (atomic_val_t)k_uptime_get_32());
    k_sem_reset(&power_awake_sem);

    LOG_INF("Power manager started (timeout %d ms)", POWER_TIMEOUT_MS);
}

void power_thread_notify_activity(void)
{
    bool was_eco;

    if (!power_thread_started) {
        return;
    }

    (void)atomic_set(&last_activity_ms, (atomic_val_t)k_uptime_get_32());
    was_eco = atomic_cas(&eco_mode, 1, 0);
    if (was_eco) {
        k_sem_give(&power_awake_sem);
    }
}

bool power_thread_is_eco_mode(void)
{
    uint32_t now_ms;
    uint32_t last_ms;

    if (!power_thread_started) {
        return false;
    }

    if (atomic_get(&eco_mode) != 0) {
        return true;
    }

    now_ms = k_uptime_get_32();
    last_ms = (uint32_t)atomic_get(&last_activity_ms);

    if ((int32_t)(now_ms - last_ms) >= POWER_TIMEOUT_MS) {
        if (atomic_cas(&eco_mode, 0, 1)) {
            k_sem_reset(&power_awake_sem);
            LOG_INF("Enter ECO mode (%d ms inactivity)", POWER_TIMEOUT_MS);
        }
        return true;
    }

    return false;
}

bool power_thread_wait_until_awake(k_timeout_t timeout)
{
    if (!power_thread_started) {
        return false;
    }

    if (!power_thread_is_eco_mode()) {
        return true;
    }

    return k_sem_take(&power_awake_sem, timeout) == 0;
}
