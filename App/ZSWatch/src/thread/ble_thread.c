#include "ble_thread.h"
#include "shared_data.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ble/ble_service.h"

LOG_MODULE_REGISTER(ble_thread, LOG_LEVEL_INF);

/* ── Thread parameters ─────────────────────────────────────────── */
#define BLE_STACK_SIZE   1024
#define BLE_PRIORITY     7

/* ════════════════════════════════════════════════════════════════
 *  Thread function
 *
 *  Flow:
 *    wait(sem_ble_notify)           ← fired by processing_thread
 *    → skip if no peer connected
 *    → lock mutex_data, copy ble_data
 *    → unlock mutex_data
 *    → ble_service_update()
 * ════════════════════════════════════════════════════════════════ */
static void ble_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    struct ble_sensor_data data;

    while (1) {
        k_sem_take(&sem_ble_notify, K_FOREVER);

        if (!ble_service_is_connected()) {
            continue;
        }

        k_mutex_lock(&mutex_data, K_FOREVER);
        data = g_processed.ble_data;
        k_mutex_unlock(&mutex_data);

        ble_service_update(&data);
    }
}

K_THREAD_DEFINE(ble_tid, BLE_STACK_SIZE,
                ble_thread_fn, NULL, NULL, NULL,
                BLE_PRIORITY, 0, 0);

/* ════════════════════════════════════════════════════════════════
 *  Public init
 * ════════════════════════════════════════════════════════════════ */
int ble_thread_init(void)
{
    int err = ble_service_init();
    if (err) {
        LOG_ERR("ble_service_init failed: %d", err);
        return err;
    }
    LOG_INF("BLE service initialised");
    return 0;
}
