#include "processing_thread.h"
#include "shared_data.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <math.h>

#include "processing/step_counter.h"
#include "processing/distance.h"
#include "processing/floor_counter.h"
#include "processing/calories.h"
#include "processing/activity.h"
#include "processing/compass.h"
#include "processing/altimeter.h"
#include "processing/fusion.h"
#include "processing/fall_detector.h"
#include "processing/weather.h"

LOG_MODULE_REGISTER(processing_thread, LOG_LEVEL_INF);

/* ── Thread parameters ─────────────────────────────────────────── */
#define PROCESSING_STACK_SIZE   2048
#define PROCESSING_PRIORITY     6

/* ── Processing module instances (private to this thread) ──────── */
static StepCounter    g_step;
static DistanceCounter g_dist;
static FloorCounter   g_floor;
static CaloriesCounter g_cal;
static Compass        g_compass;
static Altimeter      g_altimeter;
static FallDetector   g_fall;

/* ── Internal state ────────────────────────────────────────────── */
static uint32_t prev_steps;
static double   prev_distance_m;

/* ════════════════════════════════════════════════════════════════
 *  Helpers: pack sensor_value arrays to BLE int16 (mg / mGauss)
 * ════════════════════════════════════════════════════════════════ */
static inline int16_t sv_to_mg(const struct sensor_value *sv)
{
    /* m/s² → mg : divide by g (9.80665), multiply by 1000 */
    double ms2 = sensor_value_to_double(sv);
    return (int16_t)(ms2 / 9.80665 * 1000.0);
}

static inline int16_t sv_to_mgauss(const struct sensor_value *sv)
{
    /* µT → mGauss : 1 µT = 10 mGauss */
    double ut = sensor_value_to_double(sv);
    return (int16_t)(ut * 10.0);
}

/* ════════════════════════════════════════════════════════════════
 *  Thread function
 *
 *  Flow:
 *    wait(sem_sensor_ready)         ← fired by sensor_thread
 *    → copy g_raw with mutex
 *    → run all processing algorithms
 *    → write g_processed with mutex
 *    → give(sem_ble_notify)         → wakes ble_thread
 * ════════════════════════════════════════════════════════════════ */
static void processing_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    /* local copy of raw data to minimise mutex hold time */
    struct watch_raw_data raw;

    while (1) {
        k_sem_take(&sem_sensor_ready, K_FOREVER);

        /* ── Snapshot raw data ─────────────────────────────────── */
        k_mutex_lock(&mutex_data, K_FOREVER);
        raw = g_raw;
        k_mutex_unlock(&mutex_data);

        /* ── Set altimeter reference on first valid pressure ────── */
        if (!altimeter_has_reference(&g_altimeter) && raw.pressure_kpa > 0.0) {
            altimeter_set_reference(&g_altimeter, raw.pressure_kpa);
            LOG_INF("Altimeter reference set: %.2f kPa", raw.pressure_kpa);
        }

        /* ── Step counter ─────────────────────────────────────── */
        step_counter_update(&g_step, raw.accel);
        uint32_t steps = step_counter_get_steps(&g_step);
        uint32_t new_steps = steps - prev_steps;

        /* ── Activity classification ──────────────────────────── */
        activity_t activity = activity_update(raw.accel);

        /* ── Distance ─────────────────────────────────────────── */
        distance_add_steps(&g_dist, new_steps);
        double dist_m = distance_get(&g_dist);
        double delta_dist = dist_m - prev_distance_m;

        /* ── Calories ─────────────────────────────────────────── */
        if (delta_dist > 0.0) {
            calories_add_distance(&g_cal, delta_dist, activity);
        }

        /* ── Altimeter / floor counter ────────────────────────── */
        altimeter_update(&g_altimeter, raw.pressure_kpa);
        double alt_m = altimeter_get_altitude(&g_altimeter);
        floor_counter_update(&g_floor, alt_m);

        /* ── Compass ──────────────────────────────────────────── */
        compass_update(&g_compass, raw.magn);

        /* ── Sensor fusion ────────────────────────────────────── */
        Orientation orient = fusion_update(raw.accel, raw.gyro,
                                           raw.magn, 100U);

        /* ── Fall detector ────────────────────────────────────── */
        double ax = sensor_value_to_double(&raw.accel[0]);
        double ay = sensor_value_to_double(&raw.accel[1]);
        double az = sensor_value_to_double(&raw.accel[2]);
        fall_detector_update(&g_fall, ax, ay, az);
        bool fell = fall_detector_get_and_clear(&g_fall);

        /* ── Weather trend ────────────────────────────────────── */
        weather_trend_t weather = weather_update(raw.pressure_kpa);

        /* ── Reset the sleep timer on any detected motion ──────── */
        if (new_steps > 0 || fell) {
            shared_data_sleep_timer_reset();
        }

        /* ── Update state for next iteration ──────────────────── */
        prev_steps      = steps;
        prev_distance_m = dist_m;

        /* ── Pack BLE data ────────────────────────────────────── */
        struct ble_sensor_data ble = {
            /* Temperature: 0.01 °C units (int16) */
            .temperature = (int16_t)(raw.temperature_c * 100.0),
            /* Humidity: 0.01 % units (uint16) */
            .humidity    = (uint16_t)(raw.humidity_pct  * 100.0),
            /* Pressure: 0.1 Pa units (uint32): kPa × 10000 */
            .pressure    = (uint32_t)(raw.pressure_kpa  * 10000.0),
            /* Accel: mg */
            .accel = {
                sv_to_mg(&raw.accel[0]),
                sv_to_mg(&raw.accel[1]),
                sv_to_mg(&raw.accel[2]),
            },
            /* Magn: mGauss */
            .magn = {
                sv_to_mgauss(&raw.magn[0]),
                sv_to_mgauss(&raw.magn[1]),
                sv_to_mgauss(&raw.magn[2]),
            },
            .steps = steps,
        };

        /* ── Write processed + ble data ────────────────────────── */
        k_mutex_lock(&mutex_data, K_FOREVER);

        g_processed.steps        = steps;
        g_processed.distance_m   = dist_m;
        g_processed.floors       = floor_counter_get(&g_floor);
        g_processed.calories     = calories_get(&g_cal);
        g_processed.activity     = activity;
        g_processed.heading      = compass_get_heading(&g_compass);
        g_processed.altitude_m   = alt_m;
        g_processed.orientation  = orient;
        g_processed.fall_detected = fell;
        g_processed.weather      = weather;
        g_processed.ble_data     = ble;

        k_mutex_unlock(&mutex_data);

        /* ── Signal BLE thread ─────────────────────────────────── */
        k_sem_give(&sem_ble_notify);
    }
}

K_THREAD_DEFINE(processing_tid, PROCESSING_STACK_SIZE,
                processing_thread_fn, NULL, NULL, NULL,
                PROCESSING_PRIORITY, 0, 0);

/* ════════════════════════════════════════════════════════════════
 *  Public init
 * ════════════════════════════════════════════════════════════════ */
void processing_thread_init(void)
{
    step_counter_init(&g_step);
    distance_init(&g_dist, USER_STRIDE_M);
    floor_counter_init(&g_floor);
    calories_init(&g_cal, USER_WEIGHT_KG);
    compass_init(&g_compass);
    altimeter_init(&g_altimeter);
    fall_detector_init(&g_fall);
    fusion_init();

    prev_steps      = 0;
    prev_distance_m = 0.0;

    LOG_INF("Processing modules initialised");
}
