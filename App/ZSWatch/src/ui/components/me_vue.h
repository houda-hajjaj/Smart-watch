/**
 * @file me_vue.h
 * @brief Gestionnaire d'écrans LVGL - transitions, mises à jour, dispatch événements
 *
 * Orchestre les écrans, applique les transitions, met à jour les widgets.
 * Envoie les événements utilisateur au controller.
 */

#ifndef ME_VUE_H
#define ME_VUE_H

#include <stdint.h>
#include <stdbool.h>

#include "view_events.h"

#ifdef __cplusplus
extern "C" {
#endif

struct lv_obj;
struct lv_display;

typedef enum {
    VIEW_SCREEN_NONE = 0,
    VIEW_SCREEN_SPLASH,
    VIEW_SCREEN_HOME,
    VIEW_SCREEN_SETTINGS,
    VIEW_SCREEN_NOTIFICATIONS,
    VIEW_SCREEN_SENSORS,
    VIEW_SCREEN_ACTIVITY,
    VIEW_SCREEN_BLE,
    VIEW_SCREEN_COUNT
} view_screen_id_t;

typedef enum {
    VIEW_TRANSITION_NONE,
    VIEW_TRANSITION_FADE,
    VIEW_TRANSITION_SLIDE_LEFT,
    VIEW_TRANSITION_SLIDE_RIGHT,
    VIEW_TRANSITION_SLIDE_UP,
    VIEW_TRANSITION_SLIDE_DOWN
} view_transition_t;

typedef struct view_model_data_t {
    uint8_t  hour, minute, second, day, month, year;
    uint8_t  battery_percent;
    bool     battery_charging;
    bool     ble_connected;
    uint32_t ble_tx_bytes_per_sec;
    int8_t   ble_rssi_dbm;
    bool     notifications_pending;
    uint32_t steps;
    float    distance_km;
    float    calories_kcal;
    float    temperature_c;
    float    humidity_pct;
    int      pressure_hpa;
    float    heading_deg;
    uint8_t  activity;
    uint8_t  weather_trend;
    float    altitude_m;
    int      floor_count;
    bool     fall_detected;
} view_model_data_t;

typedef void (*view_event_cb_t)(uint32_t event_id,
                                const view_event_data_t *event_data,
                                void *user_data);

typedef struct {
    view_event_cb_t event_callback;
    void           *event_user_data;
} view_config_t;

int ME_VUE_init(const view_config_t *config);
void ME_VUE_deinit(void);
void ME_VUE_process(void);
void ME_VUE_update(const view_model_data_t *model);
void ME_VUE_send_event(uint32_t event_id);
void ME_VUE_send_event_data(uint32_t event_id,
                                         const view_event_data_t *event_data);
void ME_VUE_show_next_screen(void);
void ME_VUE_show_screen(view_screen_id_t screen_id,
                                     view_transition_t transition,
                                     uint32_t delay_ms);
view_screen_id_t ME_VUE_get_current_screen(void);
void *ME_VUE_get_display(void);

#ifdef __cplusplus
}
#endif

#endif /* ME_VUE_H */
