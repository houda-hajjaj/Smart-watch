#include "../components/screens_internal.h"
#include "../components/styles.h"

void screen_weather_create(void)
{
    lv_obj_t *card, *label, *home_btn;

    s_weather = lv_obj_create(NULL);
    screens_apply_screen_base(s_weather);
    screens_create_status_bar(s_weather, SB_WEATHER);

    card = lv_obj_create(s_weather);
    lv_obj_add_style(card, view_style_card(), 0);
    lv_obj_set_size(card, CARD_W, CARD_H);
    lv_obj_align(card, LV_ALIGN_TOP_LEFT, 10, SB_H + 10);
    screens_make_clickable_card(card, VIEW_EVENT_NAV_HOME);
    label = lv_label_create(card);
    lv_label_set_text(label, LV_SYMBOL_IMAGE " Temp");
    screens_set_label_style(label, view_font_label(), 0x8B949E);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 8, 0);
    s_wx_temp = lv_label_create(card);
    lv_label_set_text(s_wx_temp, "-- °C");
    screens_set_label_style(s_wx_temp, view_font_title(), 0xF0F6FC);
    lv_obj_align(s_wx_temp, LV_ALIGN_RIGHT_MID, -8, 0);

    card = lv_obj_create(s_weather);
    lv_obj_add_style(card, view_style_card(), 0);
    lv_obj_set_size(card, CARD_W, CARD_H);
    lv_obj_align(card, LV_ALIGN_TOP_LEFT, 10, SB_H + 10 + CARD_H + CARD_GAP);
    screens_make_clickable_card(card, VIEW_EVENT_NAV_SENSORS);
    label = lv_label_create(card);
    lv_label_set_text(label, LV_SYMBOL_GPS " Pression");
    screens_set_label_style(label, view_font_label(), 0x8B949E);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 8, 0);
    s_wx_press = lv_label_create(card);
    lv_label_set_text(s_wx_press, "-- hPa");
    screens_set_label_style(s_wx_press, view_font_title(), 0xF0F6FC);
    lv_obj_align(s_wx_press, LV_ALIGN_RIGHT_MID, -8, 0);

    card = lv_obj_create(s_weather);
    lv_obj_add_style(card, view_style_card(), 0);
    lv_obj_set_size(card, CARD_W, CARD_H);
    lv_obj_align(card, LV_ALIGN_TOP_LEFT, 10, SB_H + 10 + 2 * (CARD_H + CARD_GAP));
    screens_make_clickable_card(card, VIEW_EVENT_NAV_ACTIVITY);
    label = lv_label_create(card);
    lv_label_set_text(label, LV_SYMBOL_TINT " Humidite");
    screens_set_label_style(label, view_font_label(), 0x8B949E);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 8, 0);
    s_wx_humid = lv_label_create(card);
    lv_label_set_text(s_wx_humid, "-- %");
    screens_set_label_style(s_wx_humid, view_font_title(), 0xF0F6FC);
    lv_obj_align(s_wx_humid, LV_ALIGN_RIGHT_MID, -8, 0);

    home_btn = screens_create_nav_button(s_weather, LV_SYMBOL_HOME, 0x58A6FF, VIEW_EVENT_NAV_HOME);
    lv_obj_align(home_btn, LV_ALIGN_TOP_LEFT, 248, 88);

    s_wx_trend = lv_label_create(s_weather);
    lv_label_set_text(s_wx_trend, "Tendance: --");
    screens_set_label_style(s_wx_trend, view_font_label(), 0x8B949E);
    lv_obj_align(s_wx_trend, LV_ALIGN_BOTTOM_LEFT, 10, -40);

    s_wx_alt = lv_label_create(s_weather);
    lv_label_set_text(s_wx_alt, "Altitude: -- m");
    screens_set_label_style(s_wx_alt, view_font_label(), 0x8B949E);
    lv_obj_align(s_wx_alt, LV_ALIGN_BOTTOM_LEFT, 10, -26);

    s_wx_floor = lv_label_create(s_weather);
    lv_label_set_text(s_wx_floor, "Etages: --");
    screens_set_label_style(s_wx_floor, view_font_label(), 0x8B949E);
    lv_obj_align(s_wx_floor, LV_ALIGN_BOTTOM_LEFT, 10, -12);
}
