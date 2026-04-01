#include "thread/ui_thread.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <lvgl.h>

#include "ble/ble_service.h"
#include "input/buttons.h"
#include "rtc/rtc.h"
#include "thread/data_init_thread.h"
#include "thread/power_thread.h"
#include "thread/sampling_thread.h"
#include "ui/ui.h"

LOG_MODULE_REGISTER(ui_thread, LOG_LEVEL_INF);

#define UI_THREAD_STACK_SIZE 4096
#define UI_THREAD_PRIORITY 2
#define UI_THREAD_PERIOD_MS 10
#define UI_MODEL_REFRESH_MS 200

K_THREAD_STACK_DEFINE(ui_thread_stack, UI_THREAD_STACK_SIZE);
static struct k_thread ui_thread_data;
static bool ui_thread_started;
static lv_obj_t *sleep_screen;

static void ui_thread_sleep_screen_create(void)
{
    if (sleep_screen != NULL) {
        return;
    }

    sleep_screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(sleep_screen);
    lv_obj_set_style_bg_color(sleep_screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(sleep_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(sleep_screen, 0, 0);
    lv_obj_clear_flag(sleep_screen, LV_OBJ_FLAG_SCROLLABLE);
}

static void ui_thread_handle_button_events(void)
{
    uint32_t button_events = buttons_consume_events();

    if ((button_events & BUTTON_EVENT_NEXT_SCENE) != 0U) {
        ME_VUE_show_next_screen();
    }
}

static void ui_thread_refresh_model(view_model_data_t *model)
{
    const DataInitContext *ctx;
    struct rtc_time now;
    SamplingData sample;

    if (!model) {
        return;
    }

    ctx = data_init_thread_get_context();
    if (ctx && ctx->rtc_ready && watch_rtc_get((WatchRTC *)&ctx->rtc, &now) == 0) {
        model->hour = (uint8_t)now.tm_hour;
        model->minute = (uint8_t)now.tm_min;
        model->second = (uint8_t)now.tm_sec;
        model->day = (uint8_t)now.tm_mday;
        model->month = (uint8_t)(now.tm_mon + 1);
        model->year = (uint8_t)(now.tm_year % 100);
    }

    if (sampling_thread_get_latest_copy(&sample)) {
        model->steps = sample.ble_data.steps;
        model->temperature_c = sample.temperature_c;
        model->humidity_pct = sample.humidity_pct;
        model->pressure_hpa = sample.pressure_hpa;
        model->distance_km = sample.distance_km;
        model->calories_kcal = sample.calories_kcal;
        model->heading_deg = sample.heading_deg;
        model->activity = sample.activity;
        model->weather_trend = sample.weather_trend;
        model->altitude_m = sample.altitude_m;
        model->floor_count = sample.floor_count;
        model->fall_detected = sample.fall_detected;
    }

    model->ble_connected = ble_service_is_connected();
    model->battery_percent = 100U;
    model->battery_charging = false;
    model->ble_rssi_dbm = 0;
}

static void ui_thread_entry(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    view_model_data_t model = {0};
    int64_t last_model_refresh_ms = 0;
    bool display_sleeping = false;
    int err;

    if (!data_init_thread_wait_done(K_FOREVER)) {
        LOG_ERR("UI thread aborted: data init not completed");
        return;
    }

    err = ui_init_with_config(NULL);
    while (err != 0) {
        LOG_ERR("UI init failed (err %d), retry in 500 ms", err);
        k_sleep(K_MSEC(500));
        err = ui_init_with_config(NULL);
    }

    ui_thread_sleep_screen_create();

    if (!device_is_ready(display_dev)) {
        LOG_ERR("UI thread aborted: display device not ready");
        return;
    }

    /* Flush the first frame, then unblank the panel. */
    ui_process();
    err = display_blanking_off(display_dev);
    if (err != 0) {
        LOG_WRN("display_blanking_off failed (err %d)", err);
    }

    LOG_INF("UI thread started");

    while (1) {
        int64_t now_ms = k_uptime_get();
        bool eco_mode = power_thread_is_eco_mode();

        if (eco_mode && !display_sleeping) {
            LOG_INF("UI entering sleep state");
            if (sleep_screen != NULL) {
                lv_screen_load(sleep_screen);
                ui_process();
            }
            err = display_blanking_on(display_dev);
            if (err != 0) {
                LOG_WRN("display_blanking_on failed (err %d)", err);
            }
            display_sleeping = true;
        } else if (!eco_mode && display_sleeping) {
            LOG_INF("UI leaving sleep state");
            err = display_blanking_off(display_dev);
            if (err != 0) {
                LOG_WRN("display_blanking_off failed while waking (err %d)", err);
            }
            ME_VUE_show_screen(ME_VUE_get_current_screen(), VIEW_TRANSITION_NONE, 0);
            ui_process();
            display_sleeping = false;
        }

        ui_thread_handle_button_events();

        if (!display_sleeping && ((now_ms - last_model_refresh_ms) >= UI_MODEL_REFRESH_MS)) {
            ui_thread_refresh_model(&model);
            ui_update(&model);
            last_model_refresh_ms = now_ms;
        }

        if (!display_sleeping) {
            ui_process();
        }
        k_sleep(K_MSEC(UI_THREAD_PERIOD_MS));
    }
}

void ui_thread_start(void)
{
    if (ui_thread_started) {
        return;
    }

    ui_thread_started = true;

    k_tid_t tid = k_thread_create(&ui_thread_data,
                                  ui_thread_stack,
                                  K_THREAD_STACK_SIZEOF(ui_thread_stack),
                                  ui_thread_entry,
                                  NULL,
                                  NULL,
                                  NULL,
                                  UI_THREAD_PRIORITY,
                                  0,
                                  K_NO_WAIT);
    k_thread_name_set(tid, "ui_thread");
}
