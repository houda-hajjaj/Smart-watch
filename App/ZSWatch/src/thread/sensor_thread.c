#include "sensor_thread.h"
#include "shared_data.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>

#include "sensors/motion_sensor.h"
#include "sensors/mag_sensor.h"
#include "sensors/env_sensor.h"
#include "sensors/rtc.h"

LOG_MODULE_REGISTER(sensor_thread, LOG_LEVEL_INF);

/* ── Thread parameters ─────────────────────────────────────────── */
#define SENSOR_STACK_SIZE   1536
#define SENSOR_PRIORITY     5

/* ── Sensor instances (private to this thread) ─────────────────── */
static MotionSensor g_motion;
static MagSensor    g_mag;
static EnvSensor    g_env;
static WatchRTC     g_rtc;

/* ── Error tracking: log only on change to avoid spam ──────────── */
static int prev_err_m, prev_err_mg, prev_err_e, prev_err_r;

/* ════════════════════════════════════════════════════════════════
 *  Thread function
 *
 *  Flow:
 *    wait(sem_sample_tick)          ← fired every 100 ms by timer1
 *    → fetch all sensors
 *    → lock mutex_data
 *    → copy raw readings to g_raw
 *    → unlock mutex_data
 *    → give(sem_sensor_ready)       → wakes processing_thread
 * ════════════════════════════════════════════════════════════════ */
static void sensor_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        /* Block until the 100 ms sample tick fires */
        k_sem_take(&sem_sample_tick, K_FOREVER);

        /* ── Fetch sensor data ─────────────────────────────────── */
        int err_m  = motion_update(&g_motion);
        int err_mg = mag_update(&g_mag);
        int err_e  = env_update(&g_env);

        struct rtc_time t;
        int err_r = watch_rtc_get(&g_rtc, &t);

        /* Log uniquement si le code d'erreur change (évite le spam à 100ms) */
        if (err_m  != prev_err_m  || err_mg != prev_err_mg ||
            err_e  != prev_err_e  || err_r  != prev_err_r) {
            if (err_m || err_mg || err_e || err_r) {
                LOG_WRN("Sensor error changed: motion=%d mag=%d env=%d rtc=%d",
                        err_m, err_mg, err_e, err_r);
            } else {
                LOG_INF("All sensors OK");
            }
            prev_err_m  = err_m;
            prev_err_mg = err_mg;
            prev_err_e  = err_e;
            prev_err_r  = err_r;
        }

        /* ── Lock shared buffer and copy ──────────────────────── */
        k_mutex_lock(&mutex_data, K_FOREVER);

        for (int i = 0; i < 3; i++) {
            g_raw.accel[i] = g_motion.accel[i];
            g_raw.gyro[i]  = g_motion.gyro[i];
            g_raw.magn[i]  = g_mag.magn[i];
        }

        /* LPS22HH: SENSOR_CHAN_PRESS in kPa  */
        /* HTS221 : SENSOR_CHAN_AMBIENT_TEMP in °C, SENSOR_CHAN_HUMIDITY in % */
        g_raw.temperature_c = sensor_value_to_double(&g_env.temp_hts);
        g_raw.humidity_pct  = sensor_value_to_double(&g_env.humidity);
        g_raw.pressure_kpa  = sensor_value_to_double(&g_env.pressure);
        g_raw.time          = t;
        g_raw.timestamp_ms  = k_uptime_get_32();

        k_mutex_unlock(&mutex_data);

        /* ── Signal processing thread ──────────────────────────── */
        k_sem_give(&sem_sensor_ready);
    }
}

K_THREAD_DEFINE(sensor_tid, SENSOR_STACK_SIZE,
                sensor_thread_fn, NULL, NULL, NULL,
                SENSOR_PRIORITY, 0, 0);

/* ════════════════════════════════════════════════════════════════
 *  Public init
 * ════════════════════════════════════════════════════════════════ */
int sensor_thread_init(void)
{
    int err;

    err = motion_init(&g_motion);
    if (err) {
        LOG_ERR("motion_init failed: %d", err);
        return err;
    }

    err = mag_init(&g_mag);
    if (err) {
        LOG_ERR("mag_init failed: %d", err);
        return err;
    }

    err = env_init(&g_env);
    if (err) {
        LOG_ERR("env_init failed: %d", err);
        return err;
    }

    err = watch_rtc_init(&g_rtc);
    if (err) {
        LOG_ERR("watch_rtc_init failed: %d", err);
        return err;
    }

    LOG_INF("All sensors initialised");
    return 0;
}