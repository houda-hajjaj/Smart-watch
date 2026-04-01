#include "thread/ble_thread.h"

#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ble/ble_service.h"
#include "processing/compass.h"
#include "rtc/rtc.h"
#include "thread/data_init_thread.h"
#include "thread/sampling_thread.h"

LOG_MODULE_REGISTER(ble_thread, LOG_LEVEL_INF);

#define BLE_THREAD_STACK_SIZE 1536
#define BLE_THREAD_PRIORITY 6

K_THREAD_STACK_DEFINE(ble_thread_stack, BLE_THREAD_STACK_SIZE);
static struct k_thread ble_thread_data;
static bool ble_thread_started;
K_SEM_DEFINE(ble_update_sem, 0, 1);

static void format_two_digits(char *dst, uint8_t value)
{
    uint8_t safe_value = (value <= 99U) ? value : 99U;

    dst[0] = (char)('0' + (safe_value / 10U));
    dst[1] = (char)('0' + (safe_value % 10U));
}

static void format_four_digits(char *dst, uint16_t value)
{
    uint16_t safe_value = (value <= 9999U) ? value : 9999U;

    dst[0] = (char)('0' + ((safe_value / 1000U) % 10U));
    dst[1] = (char)('0' + ((safe_value / 100U) % 10U));
    dst[2] = (char)('0' + ((safe_value / 10U) % 10U));
    dst[3] = (char)('0' + (safe_value % 10U));
}

static void format_rtc_text(char out[BLE_RTC_TEXT_LEN],
                            uint16_t year,
                            uint8_t month,
                            uint8_t day,
                            uint8_t hour,
                            uint8_t minute,
                            uint8_t second)
{
    format_four_digits(&out[0], year);
    out[4] = '-';
    format_two_digits(&out[5], month);
    out[7] = '-';
    format_two_digits(&out[8], day);
    out[10] = ' ';
    format_two_digits(&out[11], hour);
    out[13] = ':';
    format_two_digits(&out[14], minute);
    out[16] = ':';
    format_two_digits(&out[17], second);
    out[19] = '\0';
}

static void ble_thread_entry(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    const DataInitContext *ctx;
    SamplingData sample;
    struct ble_sensor_data payload;
    struct rtc_time now;
    const char *direction;
    uint16_t rtc_year;
    uint8_t rtc_month;
    uint8_t rtc_day;
    uint8_t rtc_hour;
    uint8_t rtc_min;
    uint8_t rtc_sec;

    if (!data_init_thread_wait_ready(K_FOREVER)) {
        LOG_ERR("BLE thread aborted: data init not ready");
        return;
    }

    ctx = data_init_thread_get_context();

    LOG_INF("BLE thread started");

    while (1) {
        if (k_sem_take(&ble_update_sem, K_FOREVER) != 0) {
            continue;
        }

        if (sampling_thread_get_latest_copy(&sample) && ble_service_is_connected()) {
            payload = sample.ble_data;

            if (ctx && ctx->rtc_ready && watch_rtc_get((WatchRTC *)&ctx->rtc, &now) == 0) {
                rtc_year = (uint16_t)(now.tm_year + 1900);
                rtc_month = (uint8_t)(now.tm_mon + 1);
                rtc_day = (uint8_t)now.tm_mday;
                rtc_hour = (uint8_t)now.tm_hour;
                rtc_min = (uint8_t)now.tm_min;
                rtc_sec = (uint8_t)now.tm_sec;
                format_rtc_text(payload.rtc_text,
                                rtc_year,
                                rtc_month,
                                rtc_day,
                                rtc_hour,
                                rtc_min,
                                rtc_sec);
            } else {
                payload.rtc_text[0] = '\0';
            }

            payload.heading_tenths_deg = (uint16_t)(sample.heading_deg * 10.0f + 0.5f);
            direction = compass_direction_to_str(sample.heading_deg);
            (void)snprintf(payload.compass_direction,
                           sizeof(payload.compass_direction),
                           "%s",
                           direction ? direction : "?");

            ble_service_update(&payload);
        }
    }
}

void ble_thread_start(void)
{
    if (ble_thread_started) {
        return;
    }

    ble_thread_started = true;

    k_tid_t tid = k_thread_create(&ble_thread_data,
                                  ble_thread_stack,
                                  K_THREAD_STACK_SIZEOF(ble_thread_stack),
                                  ble_thread_entry,
                                  NULL,
                                  NULL,
                                  NULL,
                                  BLE_THREAD_PRIORITY,
                                  0,
                                  K_NO_WAIT);
    k_thread_name_set(tid, "ble_thread");
}

void ble_thread_notify_sample(uint32_t sample_count)
{
    ARG_UNUSED(sample_count);
    k_sem_give(&ble_update_sem);
}
