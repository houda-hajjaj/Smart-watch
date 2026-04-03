/**
 * @file screens_internal.h
 * @brief Déclarations internes de la Vue (widgets + helpers)
 *
 * Réservé au module View : utilisé par screens.c et screen_*.c.
 */

#ifndef ZSWATCH_VIEW_SCREENS_INTERNAL_H
#define ZSWATCH_VIEW_SCREENS_INTERNAL_H

#include <lvgl.h>
#include "view_events.h"
#include "me_vue.h"

/* Dimensions et constantes d'IHM */
#define SCR_W       320
#define SCR_H       240
#define SB_H        28
#define BTN_SZ      56
#define BTN_SM      36
#define CARD_W      200
#define CARD_H      44
#define CARD_GAP    8

enum { SB_HOME = 0, SB_WEATHER, SB_COMPASS, SB_ACTIVITY, SB_BLE, SB_COUNT };

/* Etat global des écrans (défini dans screens.c) */
extern view_config_t s_config;

extern lv_obj_t *s_splash;
extern lv_obj_t *s_home;
extern lv_obj_t *s_weather;
extern lv_obj_t *s_compass;
extern lv_obj_t *s_activity;
extern lv_obj_t *s_ble;

extern lv_obj_t *s_sb_ble[];
extern lv_obj_t *s_sb_time[];
extern lv_obj_t *s_sb_batt[];

extern lv_obj_t *s_home_hour;
extern lv_obj_t *s_home_min;
extern lv_obj_t *s_home_sec;
extern lv_obj_t *s_home_date;
extern lv_obj_t *s_home_steps;

extern lv_obj_t *s_wx_temp;
extern lv_obj_t *s_wx_press;
extern lv_obj_t *s_wx_humid;
extern lv_obj_t *s_wx_trend;
extern lv_obj_t *s_wx_alt;
extern lv_obj_t *s_wx_floor;

extern lv_obj_t *s_cmp_ring;
extern lv_obj_t *s_cmp_needle;
extern lv_obj_t *s_cmp_heading;
extern lv_obj_t *s_cmp_dir;

extern lv_obj_t *s_act_steps;
extern lv_obj_t *s_act_dist;
extern lv_obj_t *s_act_cal;
extern lv_obj_t *s_act_type;
extern lv_obj_t *s_act_fall;

extern lv_obj_t *s_ble_stat;
extern lv_obj_t *s_ble_rssi;

/* Helpers partagés (implémentés dans screens.c) */
void screens_send_event(view_event_id_t id, const view_event_data_t *event_data);
void screens_nav_event_cb(lv_event_t *e);
void screens_apply_screen_base(lv_obj_t *scr);
void screens_set_label_style(lv_obj_t *lbl, const lv_font_t *font, uint32_t color);
void screens_create_status_bar(lv_obj_t *parent, int idx);
void screens_make_clickable(lv_obj_t *obj, view_event_id_t event_id);
void screens_make_clickable_card(lv_obj_t *obj, view_event_id_t event_id);
lv_obj_t *screens_create_nav_button(lv_obj_t *parent, const char *symbol,
                                    uint32_t color, view_event_id_t event_id);
lv_obj_t *screens_create_step_icon(lv_obj_t *parent);

/* API de création d'écran (implémentée dans les fichiers par écran) */
void screen_splash_create(void);
void screen_home_create(void);
void screen_weather_create(void);
void screen_compass_create(void);
void screen_activity_create(void);
void screen_ble_create(void);

#endif /* ZSWATCH_VIEW_SCREENS_INTERNAL_H */
