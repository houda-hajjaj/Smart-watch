#include "thread/sampling_thread.h"

#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>

#include "processing/activity.h"
#include "processing/altimeter.h"
#include "processing/calories.h"
#include "processing/compass.h"
#include "processing/distance.h"
#include "processing/fall_detector.h"
#include "processing/floor_counter.h"
#include "processing/weather.h"
#include "thread/ble_thread.h"
#include "thread/data_init_thread.h"
#include "thread/power_thread.h"
#include "processing/step_counter.h"

LOG_MODULE_REGISTER(sampling_thread, LOG_LEVEL_INF);

#define SAMPLING_THREAD_STACK_SIZE 2048
#define SAMPLING_THREAD_PRIORITY 5
#define FALL_DISPLAY_HOLD_MS 5000

K_THREAD_STACK_DEFINE(sampling_stack, SAMPLING_THREAD_STACK_SIZE);
static struct k_thread sampling_thread;
static k_tid_t sampling_tid;

static struct k_timer timer1;
K_SEM_DEFINE(sampling_tick_sem, 0, 1);
K_MUTEX_DEFINE(sampling_data_mutex);

static SamplingData latest_sample;
static bool sampling_started;
static bool sampling_running;

static int16_t sensor_to_milli(const struct sensor_value *v)
{
    int64_t value = (int64_t)v->val1 * 1000 + (int64_t)v->val2 / 1000;

    if (value > INT16_MAX) {
        return INT16_MAX;
    }
    if (value < INT16_MIN) {
        return INT16_MIN;
    }

    return (int16_t)value;
}

static uint16_t sensor_to_centi_u16(const struct sensor_value *v)
{
    int64_t value = (int64_t)v->val1 * 100 + (int64_t)v->val2 / 10000;

    if (value < 0) {
        return 0;
    }
    if (value > UINT16_MAX) {
        return UINT16_MAX;
    }

    return (uint16_t)value;
}

static uint32_t sensor_to_deci_u32(const struct sensor_value *v)
{
    int64_t value = (int64_t)v->val1 * 10 + (int64_t)v->val2 / 100000;

    if (value < 0) {
        return 0;
    }
    if (value > UINT32_MAX) {
        return UINT32_MAX;
    }

    return (uint32_t)value;
}

static void timer1_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_sem_give(&sampling_tick_sem);
}

static void sampling_entry(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    if (!data_init_thread_wait_ready(K_FOREVER)) {
        LOG_ERR("Sampling thread aborted: data init not ready");
        return;
    }

    DataInitContext *ctx = data_init_thread_get_context_mutable();
    StepCounter step_counter;
    DistanceCounter distance_counter;
    CaloriesCounter calories_counter;
    Altimeter altimeter;
    FloorCounter floor_counter;
    Compass compass;
    FallDetector fall_detector;
    uint32_t prev_steps = 0U;
    int64_t fall_display_until_ms = 0;
    int64_t last_ble_notify_ms = 0;
    bool timer_running = false;

    step_counter_init(&step_counter);
    distance_init(&distance_counter, 0.75);
    calories_init(&calories_counter, 70.0);
    altimeter_init(&altimeter);
    floor_counter_init(&floor_counter);
    compass_init(&compass);
    fall_detector_init(&fall_detector);
    weather_init();

    k_timer_init(&timer1, timer1_handler, NULL);
    k_timer_start(&timer1, K_MSEC(1), K_MSEC(1));
    timer_running = true;
    sampling_running = true;
    LOG_INF("Sampling started with timer1 period 1 ms");

    while (1) {
        struct ble_sensor_data frame = {0};
        activity_t activity = ACTIVITY_UNKNOWN;
        weather_trend_t weather_trend = WEATHER_UNKNOWN;
        double temperature_c = 0.0;
        double humidity_pct = 0.0;
        double pressure_kpa = 0.0;
        double altitude_m = 0.0;
        double heading_deg = 0.0;
        bool fall_detected = false;
        uint32_t steps;

        if (power_thread_is_eco_mode()) {
            if (timer_running) {
                k_timer_stop(&timer1);
                timer_running = false;
            }

            if (!power_thread_wait_until_awake(K_FOREVER)) {
                continue;
            }

            k_sem_reset(&sampling_tick_sem);
            k_timer_start(&timer1, K_MSEC(1), K_MSEC(1));
            timer_running = true;
            continue;
        }

        if (k_sem_take(&sampling_tick_sem, K_FOREVER) != 0) {
            continue;
        }

        int64_t now_ms = k_uptime_get();

        if (motion_update(&ctx->motion) == 0) {
            step_counter_update(&step_counter, ctx->motion.accel);
            frame.accel[0] = sensor_to_milli(&ctx->motion.accel[0]);
            frame.accel[1] = sensor_to_milli(&ctx->motion.accel[1]);
            frame.accel[2] = sensor_to_milli(&ctx->motion.accel[2]);
            activity = activity_update(ctx->motion.accel);
            fall_detected = fall_detector_update(&fall_detector,
                sensor_value_to_double(&ctx->motion.accel[0]),
                sensor_value_to_double(&ctx->motion.accel[1]),
                sensor_value_to_double(&ctx->motion.accel[2]));
        }

        if (mag_update(&ctx->mag) == 0) {
            frame.magn[0] = sensor_to_milli(&ctx->mag.magn[0]);
            frame.magn[1] = sensor_to_milli(&ctx->mag.magn[1]);
            frame.magn[2] = sensor_to_milli(&ctx->mag.magn[2]);
            compass_update(&compass, ctx->motion.accel, ctx->mag.magn);
            heading_deg = compass_get_heading(&compass);
        }

        if (env_update(&ctx->env) == 0) {
            frame.temperature = sensor_to_milli(&ctx->env.temp_hts);
            frame.humidity = sensor_to_centi_u16(&ctx->env.humidity);
            frame.pressure = sensor_to_deci_u32(&ctx->env.pressure);
            temperature_c = sensor_value_to_double(&ctx->env.temp_hts);
            humidity_pct = sensor_value_to_double(&ctx->env.humidity);
            pressure_kpa = sensor_value_to_double(&ctx->env.pressure);
            weather_trend = weather_update(pressure_kpa);
            if (!altimeter_has_reference(&altimeter)) {
                altimeter_set_reference(&altimeter, pressure_kpa);
            } else {
                altimeter_update(&altimeter, pressure_kpa);
            }
            altitude_m = altimeter_get_altitude(&altimeter);
            floor_counter_update(&floor_counter, altitude_m);
        }

        steps = step_counter_get_steps(&step_counter);
        frame.steps = steps;

        if (steps > prev_steps) {
            uint32_t steps_added = steps - prev_steps;
            double distance_before = distance_get(&distance_counter);
            activity_t calorie_activity = activity;

            distance_add_steps(&distance_counter, steps_added);

            /* Counted steps mean the user is moving, even if the
             * instantaneous activity classifier still reports rest.
             */
            if (calorie_activity != ACTIVITY_RUNNING) {
                calorie_activity = ACTIVITY_WALKING;
            }

            calories_add_distance(&calories_counter,
                                  distance_get(&distance_counter) - distance_before,
                                  calorie_activity);
            prev_steps = steps;
        }

        if (fall_detected) {
            fall_display_until_ms = now_ms + FALL_DISPLAY_HOLD_MS;
        }

        k_mutex_lock(&sampling_data_mutex, K_FOREVER);
        latest_sample.ble_data = frame;
        latest_sample.temperature_c = (float)temperature_c;
        latest_sample.humidity_pct = (float)humidity_pct;
        latest_sample.pressure_hpa = (int)(pressure_kpa * 10.0 + 0.5);
        latest_sample.distance_km = (float)(distance_get(&distance_counter) / 1000.0);
        latest_sample.calories_kcal = (float)calories_get(&calories_counter);
        latest_sample.heading_deg = (float)heading_deg;
        latest_sample.activity = (uint8_t)activity;
        latest_sample.weather_trend = (uint8_t)weather_trend;
        latest_sample.altitude_m = (float)altitude_m;
        latest_sample.floor_count = floor_counter_get(&floor_counter);
        latest_sample.fall_detected = (now_ms <= fall_display_until_ms);
        latest_sample.sample_count++;
        latest_sample.timestamp_ms = now_ms;
        latest_sample.valid = true;
        k_mutex_unlock(&sampling_data_mutex);

        if ((now_ms - last_ble_notify_ms) >= BLE_THREAD_UPDATE_PERIOD_MS) {
            ble_thread_notify_sample(latest_sample.sample_count);
            last_ble_notify_ms = now_ms;
        }
    }
}

void sampling_thread_start(void)
{
    if (sampling_started) {
        return;
    }

    sampling_started = true;
    sampling_tid = k_thread_create(&sampling_thread,
                                   sampling_stack,
                                   K_THREAD_STACK_SIZEOF(sampling_stack),
                                   sampling_entry,
                                   NULL,
                                   NULL,
                                   NULL,
                                   SAMPLING_THREAD_PRIORITY,
                                   0,
                                   K_NO_WAIT);
    k_thread_name_set(sampling_tid, "sampling");
}

bool sampling_thread_is_running(void)
{
    return sampling_running;
}

const SamplingData *sampling_thread_get_latest(void)
{
    return &latest_sample;
}

bool sampling_thread_get_latest_copy(SamplingData *out)
{
    if (out == NULL) {
        return false;
    }

    k_mutex_lock(&sampling_data_mutex, K_FOREVER);
    *out = latest_sample;
    k_mutex_unlock(&sampling_data_mutex);

    return out->valid;
}
