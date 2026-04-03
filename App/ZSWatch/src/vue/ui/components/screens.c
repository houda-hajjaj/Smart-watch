/**
 * @file screens.c
 * @brief Construction et mise à jour des écrans LVGL
 *
 * Style cours MVC IoT :
 * - Ce module gère uniquement l'IHM (widgets, rendu, interactions).
 * - Les interactions sont converties en événements View -> Controller.
 * - Aucun calcul métier n'est réalisé ici.
 */


#include "screens.h"
#include "styles.h"
#include "view_events.h"
#include "screens_internal.h"
#include "thread/power_thread.h"
#include <limits.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

LOG_MODULE_REGISTER(view_screens, LOG_LEVEL_INF);

#define SWIPE_THRESHOLD_PX 30
#define SWIPE_AXIS_BIAS_PX 12

view_config_t s_config;

lv_obj_t *s_splash;
lv_obj_t *s_home;
lv_obj_t *s_weather;
lv_obj_t *s_compass;
lv_obj_t *s_activity;
lv_obj_t *s_ble;

lv_obj_t *s_sb_ble[SB_COUNT];
lv_obj_t *s_sb_time[SB_COUNT];
lv_obj_t *s_sb_batt[SB_COUNT];

lv_obj_t *s_home_hour;
lv_obj_t *s_home_min;
lv_obj_t *s_home_sec;
lv_obj_t *s_home_date;
lv_obj_t *s_home_steps;

lv_obj_t *s_wx_temp;
lv_obj_t *s_wx_press;
lv_obj_t *s_wx_humid;
lv_obj_t *s_wx_trend;
lv_obj_t *s_wx_alt;
lv_obj_t *s_wx_floor;

lv_obj_t *s_cmp_ring;
lv_obj_t *s_cmp_needle;
lv_obj_t *s_cmp_heading;
lv_obj_t *s_cmp_dir;

lv_obj_t *s_act_steps;
lv_obj_t *s_act_dist;
lv_obj_t *s_act_cal;
lv_obj_t *s_act_type;
lv_obj_t *s_act_fall;

lv_obj_t *s_ble_stat;
lv_obj_t *s_ble_rssi;

static struct {
    bool active;
    uint8_t screen_id;
    lv_point_t start;
} s_touch_gesture;

static void clear_widget_refs(void)
{
    memset(s_sb_ble, 0, sizeof(s_sb_ble));
    memset(s_sb_time, 0, sizeof(s_sb_time));
    memset(s_sb_batt, 0, sizeof(s_sb_batt));

    s_home_hour = NULL;
    s_home_min = NULL;
    s_home_sec = NULL;
    s_home_date = NULL;
    s_home_steps = NULL;

    s_wx_temp = NULL;
    s_wx_press = NULL;
    s_wx_humid = NULL;
    s_wx_trend = NULL;
    s_wx_alt = NULL;
    s_wx_floor = NULL;

    s_cmp_ring = NULL;
    s_cmp_needle = NULL;
    s_cmp_heading = NULL;
    s_cmp_dir = NULL;

    s_act_steps = NULL;
    s_act_dist = NULL;
    s_act_cal = NULL;
    s_act_type = NULL;
    s_act_fall = NULL;

    s_ble_stat = NULL;
    s_ble_rssi = NULL;
}

static lv_obj_t *screen_get_or_create(view_screen_id_t id)
{
    switch (id) {
    case VIEW_SCREEN_SPLASH:
        if (!s_splash) {
            screen_splash_create();
        }
        return s_splash;
    case VIEW_SCREEN_HOME:
    case VIEW_SCREEN_SETTINGS:
        if (!s_home) {
            screen_home_create();
        }
        return s_home;
    case VIEW_SCREEN_NOTIFICATIONS:
        if (!s_weather) {
            screen_weather_create();
        }
        return s_weather;
    case VIEW_SCREEN_SENSORS:
        if (!s_compass) {
            screen_compass_create();
        }
        return s_compass;
    case VIEW_SCREEN_ACTIVITY:
        if (!s_activity) {
            screen_activity_create();
        }
        return s_activity;
    case VIEW_SCREEN_BLE:
        if (!s_ble) {
            screen_ble_create();
        }
        return s_ble;
    case VIEW_SCREEN_NONE:
    case VIEW_SCREEN_COUNT:
    default:
        return NULL;
    }
}

static uint8_t screen_id_from_root(const lv_obj_t *root)
{
    if (root == s_splash) {
        return VIEW_SCREEN_SPLASH;
    }
    if (root == s_home) {
        return VIEW_SCREEN_HOME;
    }
    if (root == s_weather) {
        return VIEW_SCREEN_NOTIFICATIONS;
    }
    if (root == s_compass) {
        return VIEW_SCREEN_SENSORS;
    }
    if (root == s_activity) {
        return VIEW_SCREEN_ACTIVITY;
    }
    if (root == s_ble) {
        return VIEW_SCREEN_BLE;
    }

    return VIEW_SCREEN_NONE;
}

/* ─── Helpers ──────────────────────────────────────────────────────────── */

static const char *event_name(view_event_id_t id)
{
    switch (id) {
    case VIEW_EVENT_NAV_HOME:
        return "NAV_HOME";
    case VIEW_EVENT_NAV_SETTINGS:
        return "NAV_SETTINGS";
    case VIEW_EVENT_NAV_NOTIFICATIONS:
        return "NAV_NOTIFICATIONS";
    case VIEW_EVENT_NAV_SENSORS:
        return "NAV_SENSORS";
    case VIEW_EVENT_NAV_ACTIVITY:
        return "NAV_ACTIVITY";
    case VIEW_EVENT_NAV_BLE:
        return "NAV_BLE";
    case VIEW_EVENT_NAV_BACK:
        return "NAV_BACK";
    case VIEW_EVENT_NAV_NEXT:
        return "NAV_NEXT";
    case VIEW_EVENT_BUTTON_PRESSED:
        return "BUTTON_PRESSED";
    case VIEW_EVENT_BUTTON_LONG_PRESSED:
        return "BUTTON_LONG_PRESSED";
    case VIEW_EVENT_BUTTON_SELECT:
        return "BUTTON_SELECT";
    case VIEW_EVENT_BUTTON_CONFIRM:
        return "BUTTON_CONFIRM";
    case VIEW_EVENT_GESTURE_SWIPE_LEFT:
        return "GESTURE_SWIPE_LEFT";
    case VIEW_EVENT_GESTURE_SWIPE_RIGHT:
        return "GESTURE_SWIPE_RIGHT";
    case VIEW_EVENT_GESTURE_SWIPE_UP:
        return "GESTURE_SWIPE_UP";
    case VIEW_EVENT_GESTURE_SWIPE_DOWN:
        return "GESTURE_SWIPE_DOWN";
    case VIEW_EVENT_SETTING_CHANGED:
        return "SETTING_CHANGED";
    case VIEW_EVENT_BLE_TOGGLE:
        return "BLE_TOGGLE";
    case VIEW_EVENT_NOTIFICATION_DISMISS:
        return "NOTIFICATION_DISMISS";
    case VIEW_EVENT_NONE:
    default:
        return "NONE";
    }
}

static bool event_point_get(lv_event_t *e, lv_point_t *point);

static void screens_note_touch_activity(lv_event_code_t code)
{
    switch (code) {
    case LV_EVENT_PRESSED:
    case LV_EVENT_PRESSING:
    case LV_EVENT_RELEASED:
    case LV_EVENT_CLICKED:
        power_thread_notify_activity();
        break;
    default:
        break;
    }
}

static void build_touch_event_data(lv_event_t *e, view_event_data_t *event_data)
{
    lv_obj_t *target;
    lv_obj_t *screen;
    lv_area_t coords;
    lv_point_t point = { 0, 0 };

    if (!event_data) {
        return;
    }

    memset(event_data, 0, sizeof(*event_data));
    event_data->source = VIEW_EVENT_SOURCE_TOUCH;
    event_data->button_id = -1;

    target = lv_event_get_target_obj(e);
    screen = target ? lv_obj_get_screen(target) : NULL;
    event_data->screen_id = screen_id_from_root(screen);

    if (event_point_get(e, &point)) {
        event_data->x = point.x;
        event_data->y = point.y;
    }

    if (!target) {
        return;
    }

    lv_obj_get_coords(target, &coords);
    event_data->target_left = coords.x1;
    event_data->target_top = coords.y1;
    event_data->target_right = coords.x2;
    event_data->target_bottom = coords.y2;
}

static bool event_point_get(lv_event_t *e, lv_point_t *point)
{
    lv_indev_t *indev;

    if (!point) {
        return false;
    }

    indev = lv_event_get_indev(e);
    if (!indev) {
        indev = lv_indev_active();
    }

    if (!indev) {
        return false;
    }

    lv_indev_get_point(indev, point);
    return true;
}

static void screens_touch_trace_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    view_event_data_t event_data;
    int16_t dx;
    int16_t dy;
    view_event_id_t gesture = VIEW_EVENT_NONE;

    build_touch_event_data(e, &event_data);
    screens_note_touch_activity(code);

    if (code == LV_EVENT_PRESSED) {
        s_touch_gesture.active = true;
        s_touch_gesture.screen_id = event_data.screen_id;
        s_touch_gesture.start.x = event_data.x;
        s_touch_gesture.start.y = event_data.y;

        LOG_INF("Touch screen=%u x=%d y=%d",
                (unsigned)event_data.screen_id,
                event_data.x,
                event_data.y);
        return;
    }

    if (code == LV_EVENT_PRESSING) {
        return;
    }

    if (!s_touch_gesture.active) {
        return;
    }

    if (code == LV_EVENT_RELEASED) {
        dx = event_data.x - s_touch_gesture.start.x;
        dy = event_data.y - s_touch_gesture.start.y;

        if ((LV_ABS(dx) >= SWIPE_THRESHOLD_PX) &&
            (LV_ABS(dx) > (LV_ABS(dy) + SWIPE_AXIS_BIAS_PX))) {
            gesture = (dx > 0) ? VIEW_EVENT_GESTURE_SWIPE_RIGHT
                               : VIEW_EVENT_GESTURE_SWIPE_LEFT;
        } else if ((LV_ABS(dy) >= SWIPE_THRESHOLD_PX) &&
                   (LV_ABS(dy) > (LV_ABS(dx) + SWIPE_AXIS_BIAS_PX))) {
            gesture = (dy > 0) ? VIEW_EVENT_GESTURE_SWIPE_DOWN
                               : VIEW_EVENT_GESTURE_SWIPE_UP;
        }

        if (gesture != VIEW_EVENT_NONE) {
            event_data.screen_id = s_touch_gesture.screen_id;
            LOG_INF("Touch gesture screen=%u dx=%d dy=%d -> %s",
                    (unsigned)event_data.screen_id,
                    dx,
                    dy,
                    event_name(gesture));
            screens_send_event(gesture, &event_data);
        }
    }

    if ((code == LV_EVENT_RELEASED) || (code == LV_EVENT_PRESS_LOST)) {
        s_touch_gesture.active = false;
    }
}

void screens_send_event(view_event_id_t id, const view_event_data_t *event_data)
{
    if (id == VIEW_EVENT_NONE) return;
    if (s_config.event_callback) {
        s_config.event_callback((uint32_t)id, event_data, s_config.event_user_data);
    }
}

void screens_nav_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    view_event_id_t event_id = (view_event_id_t)(uintptr_t)lv_event_get_user_data(e);
    view_event_data_t event_data;

    build_touch_event_data(e, &event_data);
    screens_note_touch_activity(code);

    if (code == LV_EVENT_PRESSED) {
        LOG_INF("Touch target screen=%u x=%d y=%d -> %s",
                (unsigned)event_data.screen_id,
                event_data.x,
                event_data.y,
                event_name(event_id));
        return;
    }

    if (code != LV_EVENT_CLICKED) {
        return;
    }

    LOG_INF("Touch click screen=%u x=%d y=%d box=[%d,%d,%d,%d] -> %s",
            (unsigned)event_data.screen_id,
            event_data.x,
            event_data.y,
            event_data.target_left,
            event_data.target_top,
            event_data.target_right,
            event_data.target_bottom,
            event_name(event_id));

    screens_send_event(event_id, &event_data);
}

void screens_apply_screen_base(lv_obj_t *scr)
{
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(scr, view_style_screen(), 0);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr, screens_touch_trace_cb, LV_EVENT_ALL, NULL);
}

void screens_set_label_style(lv_obj_t *lbl, const lv_font_t *font, uint32_t color)
{
    lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(color), 0);
    lv_obj_set_style_text_opa(lbl, LV_OPA_COVER, 0);
}

void screens_create_status_bar(lv_obj_t *parent, int idx)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_t *next_btn;
    lv_obj_t *next_lbl;
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, SCR_W, SB_H);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x161B22), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_remove_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    s_sb_ble[idx] = lv_label_create(bar);
    lv_label_set_text(s_sb_ble[idx], LV_SYMBOL_WIFI);
    screens_set_label_style(s_sb_ble[idx], view_font_label(), 0x58A6FF);
    lv_obj_align(s_sb_ble[idx], LV_ALIGN_LEFT_MID, 8, 0);
    screens_make_clickable(s_sb_ble[idx], VIEW_EVENT_NAV_BLE);

    s_sb_time[idx] = lv_label_create(bar);
    lv_label_set_text(s_sb_time[idx], "12:00:00");
    screens_set_label_style(s_sb_time[idx], view_font_title(), 0xF0F6FC);
    lv_obj_align(s_sb_time[idx], LV_ALIGN_CENTER, 0, 0);
    screens_make_clickable(s_sb_time[idx], VIEW_EVENT_NAV_HOME);

    s_sb_batt[idx] = lv_label_create(bar);
    lv_label_set_text(s_sb_batt[idx], LV_SYMBOL_CHARGE " 100%");
    screens_set_label_style(s_sb_batt[idx], view_font_label(), 0x3FB950);
    lv_obj_align(s_sb_batt[idx], LV_ALIGN_RIGHT_MID, -52, 0);

    next_btn = lv_obj_create(bar);
    lv_obj_remove_style_all(next_btn);
    lv_obj_set_size(next_btn, 34, 22);
    lv_obj_align(next_btn, LV_ALIGN_RIGHT_MID, -8, 0);
    lv_obj_set_style_bg_color(next_btn, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(next_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(next_btn, 8, 0);
    screens_make_clickable(next_btn, VIEW_EVENT_NAV_NEXT);

    next_lbl = lv_label_create(next_btn);
    lv_label_set_text(next_lbl, LV_SYMBOL_RIGHT);
    screens_set_label_style(next_lbl, view_font_label(), 0x58A6FF);
    lv_obj_center(next_lbl);
}

void screens_make_clickable(lv_obj_t *obj, view_event_id_t event_id)
{
    if (!obj || event_id == VIEW_EVENT_NONE) {
        return;
    }

    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(obj, screens_nav_event_cb, LV_EVENT_ALL,
                        (void *)(uintptr_t)event_id);
}

void screens_make_clickable_card(lv_obj_t *obj, view_event_id_t event_id)
{
    screens_make_clickable(obj, event_id);
    lv_obj_add_style(obj, view_style_card_pressed(), LV_STATE_PRESSED);
}

lv_obj_t *screens_create_nav_button(lv_obj_t *parent, const char *symbol, uint32_t color,
                                    view_event_id_t event_id)
{
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_t *lbl;
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, BTN_SZ, BTN_SZ);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn, 14, 0);
    screens_make_clickable(btn, event_id);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, symbol);
    screens_set_label_style(lbl, view_font_title(), color);
    lv_obj_center(lbl);
    return btn;
}

lv_obj_t *screens_create_step_icon(lv_obj_t *parent)
{
    lv_obj_t *icon = lv_obj_create(parent);
    lv_obj_t *p;
    lv_obj_remove_style_all(icon);
    lv_obj_set_size(icon, 18, 14);
    lv_obj_set_style_bg_opa(icon, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(icon, 0, 0);
    lv_obj_remove_flag(icon, LV_OBJ_FLAG_SCROLLABLE);
    p = lv_obj_create(icon);
    lv_obj_remove_style_all(p);
    lv_obj_set_size(p, 10, 6);
    lv_obj_set_pos(p, 1, 5);
    lv_obj_set_style_bg_color(p, lv_color_hex(0xFF2A2A), 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(p, 3, 0);
    p = lv_obj_create(icon);
    lv_obj_remove_style_all(p);
    lv_obj_set_size(p, 6, 5);
    lv_obj_set_pos(p, 10, 0);
    lv_obj_set_style_bg_color(p, lv_color_hex(0xFF2A2A), 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(p, LV_RADIUS_CIRCLE, 0);
    return icon;
}

/* ─── Init / Deinit ────────────────────────────────────────────────────── */

/* ─── Init / Deinit ────────────────────────────────────────────────────── */

int view_screens_init(const view_config_t *config)
{
    clear_widget_refs();
    view_styles_init();
    if (config) {
        s_config = *config;
    } else {
        s_config.event_callback = 0;
        s_config.event_user_data = 0;
    }

    screen_splash_create();
    screen_home_create();
    return (s_splash && s_home) ? 0 : -1;
}

void view_screens_deinit(void)
{
    if (s_splash) lv_obj_del(s_splash);
    if (s_home) lv_obj_del(s_home);
    if (s_weather) lv_obj_del(s_weather);
    if (s_compass) lv_obj_del(s_compass);
    if (s_activity) lv_obj_del(s_activity);
    if (s_ble) lv_obj_del(s_ble);
    s_splash = NULL;
    s_home = NULL;
    s_weather = NULL;
    s_compass = NULL;
    s_activity = NULL;
    s_ble = NULL;
    clear_widget_refs();
}

lv_obj_t *view_screens_get(view_screen_id_t id)
{
    return screen_get_or_create(id);
}

/* ─── Mise à jour model ────────────────────────────────────────────────── */

void view_screens_update(const view_model_data_t *model)
{
    char buf[32];
    int i;

    if (!model) return;

    for (i = 0; i < SB_COUNT; i++) {
        if (s_sb_time[i]) {
            lv_snprintf(buf, sizeof(buf), "%02u:%02u:%02u",
                        (unsigned)model->hour, (unsigned)model->minute,
                        (unsigned)model->second);
            lv_label_set_text(s_sb_time[i], buf);
        }
        if (s_sb_batt[i]) {
            lv_snprintf(buf, sizeof(buf), LV_SYMBOL_CHARGE " %u%%",
                        (unsigned)model->battery_percent);
            lv_label_set_text(s_sb_batt[i], buf);
        }
        if (s_sb_ble[i]) {
            lv_label_set_text(s_sb_ble[i],
                              model->ble_connected ? LV_SYMBOL_WIFI : LV_SYMBOL_WARNING);
            lv_obj_set_style_text_color(s_sb_ble[i],
                lv_color_hex(model->ble_connected ? 0x58A6FF : 0x8B949E), 0);
        }
    }

    if (s_home_hour) {
        lv_snprintf(buf, sizeof(buf), "%02u", (unsigned)model->hour);
        lv_label_set_text(s_home_hour, buf);
    }
    if (s_home_min) {
        lv_snprintf(buf, sizeof(buf), "%02u", (unsigned)model->minute);
        lv_label_set_text(s_home_min, buf);
    }
    if (s_home_sec) {
        lv_snprintf(buf, sizeof(buf), "%02u", (unsigned)model->second);
        lv_label_set_text(s_home_sec, buf);
    }
    if (s_home_date) {
        lv_snprintf(buf, sizeof(buf), "%02u/%02u/%02u",
                    (unsigned)model->day, (unsigned)model->month, (unsigned)model->year);
        lv_label_set_text(s_home_date, buf);
    }
    if (s_home_steps) {
        lv_snprintf(buf, sizeof(buf), "%u", (unsigned)model->steps);
        lv_label_set_text(s_home_steps, buf);
    }

    if (s_wx_temp) {
        lv_snprintf(buf, sizeof(buf), "%d °C", (int)model->temperature_c);
        lv_label_set_text(s_wx_temp, buf);
    }
    if (s_wx_press) {
        lv_snprintf(buf, sizeof(buf), "%d hPa", model->pressure_hpa);
        lv_label_set_text(s_wx_press, buf);
    }
    if (s_wx_humid) {
        lv_snprintf(buf, sizeof(buf), "%d %%", (int)model->humidity_pct);
        lv_label_set_text(s_wx_humid, buf);
    }
    if (s_wx_trend) {
        const char *t = "?";
        if (model->weather_trend == 1) t = "Stable";
        else if (model->weather_trend == 2) t = "Hausse";
        else if (model->weather_trend == 3) t = "Baisse";
        lv_snprintf(buf, sizeof(buf), "Tendance: %s", t);
        lv_label_set_text(s_wx_trend, buf);
    }
    if (s_wx_alt) {
        lv_snprintf(buf, sizeof(buf), "Alt: %d m", (int)(model->altitude_m + 0.5f));
        lv_label_set_text(s_wx_alt, buf);
    }
    if (s_wx_floor) {
        lv_snprintf(buf, sizeof(buf), "Etages: %d", model->floor_count);
        lv_label_set_text(s_wx_floor, buf);
    }

    if (s_cmp_heading) {
        lv_snprintf(buf, sizeof(buf), "%d\xC2\xB0", (int)(model->heading_deg + 0.5f));
        lv_label_set_text(s_cmp_heading, buf);
    }
    if (s_cmp_dir) {
        const char *d = "N";
        float h = model->heading_deg;
        if (h >= 22.5f && h < 67.5f) d = "NE";
        else if (h >= 67.5f && h < 112.5f) d = "E";
        else if (h >= 112.5f && h < 157.5f) d = "SE";
        else if (h >= 157.5f && h < 202.5f) d = "S";
        else if (h >= 202.5f && h < 247.5f) d = "SO";
        else if (h >= 247.5f && h < 292.5f) d = "O";
        else if (h >= 292.5f && h < 337.5f) d = "NO";
        lv_label_set_text(s_cmp_dir, d);
    }
    if (s_cmp_needle) {
        lv_obj_set_style_transform_rotation(s_cmp_needle,
                                            (int32_t)(model->heading_deg * 10.0f), 0);
    }

    if (s_act_steps) {
        lv_snprintf(buf, sizeof(buf), "%u", (unsigned)model->steps);
        lv_label_set_text(s_act_steps, buf);
    }
    if (s_act_dist) {
        int dm = (int)(model->distance_km * 100.0f + 0.5f);
        lv_snprintf(buf, sizeof(buf), "%d.%02d km", dm / 100, dm % 100);
        lv_label_set_text(s_act_dist, buf);
    }
    if (s_act_cal) {
        int deci_kcal = (int)(model->calories_kcal * 10.0f + 0.5f);
        lv_snprintf(buf, sizeof(buf), "%d.%d kcal", deci_kcal / 10, deci_kcal % 10);
        lv_label_set_text(s_act_cal, buf);
    }
    if (s_act_type) {
        const char *a = "?";
        if (model->activity == 1) a = "Repos";
        else if (model->activity == 2) a = "Marche";
        else if (model->activity == 3) a = "Course";
        lv_snprintf(buf, sizeof(buf), "Type: %s", a);
        lv_label_set_text(s_act_type, buf);
    }
    if (s_act_fall) {
        lv_label_set_text(s_act_fall,
                          model->fall_detected ? "! CHUTE !" : "");
    }

    if (s_ble_stat) {
        lv_label_set_text(s_ble_stat,
                          model->ble_connected ? "Connecte" : "Deconnecte");
    }
    if (s_ble_rssi) {
        if (model->ble_connected && model->ble_rssi_dbm != INT8_MAX)
            lv_snprintf(buf, sizeof(buf), "RSSI: %d dBm", (int)model->ble_rssi_dbm);
        else
            lv_snprintf(buf, sizeof(buf), "RSSI: -- dBm");
        lv_label_set_text(s_ble_rssi, buf);
    }
}
