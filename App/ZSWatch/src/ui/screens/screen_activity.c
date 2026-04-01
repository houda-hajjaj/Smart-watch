#include "../components/screens_internal.h"
#include "../components/styles.h"

void screen_activity_create(void)
{
    lv_obj_t *card, *label, *home_btn;

    s_activity = lv_obj_create(NULL);
    screens_apply_screen_base(s_activity);
    screens_create_status_bar(s_activity, SB_ACTIVITY);

    card = lv_obj_create(s_activity);
    lv_obj_add_style(card, view_style_card(), 0);
    lv_obj_set_size(card, CARD_W, CARD_H);
    lv_obj_align(card, LV_ALIGN_TOP_LEFT, 10, SB_H + 10);
    screens_make_clickable_card(card, VIEW_EVENT_NAV_HOME);

    label = lv_label_create(card);
    lv_label_set_text(label, "Pas");
    screens_set_label_style(label, view_font_label(), 0x8B949E);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 30, 0);

    screens_create_step_icon(card);

    s_act_steps = lv_label_create(card);
    lv_label_set_text(s_act_steps, "--");
    screens_set_label_style(s_act_steps, view_font_title(), 0xF0F6FC);
    lv_obj_align(s_act_steps, LV_ALIGN_RIGHT_MID, -8, 0);

    card = lv_obj_create(s_activity);
    lv_obj_add_style(card, view_style_card(), 0);
    lv_obj_set_size(card, CARD_W, CARD_H);
    lv_obj_align(card, LV_ALIGN_TOP_LEFT, 10, SB_H + 10 + CARD_H + CARD_GAP);
    screens_make_clickable_card(card, VIEW_EVENT_NAV_SENSORS);
    label = lv_label_create(card);
    lv_label_set_text(label, LV_SYMBOL_GPS " Distance");
    screens_set_label_style(label, view_font_label(), 0x8B949E);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 8, 0);
    s_act_dist = lv_label_create(card);
    lv_label_set_text(s_act_dist, "-- km");
    screens_set_label_style(s_act_dist, view_font_title(), 0xF0F6FC);
    lv_obj_align(s_act_dist, LV_ALIGN_RIGHT_MID, -8, 0);

    card = lv_obj_create(s_activity);
    lv_obj_add_style(card, view_style_card(), 0);
    lv_obj_set_size(card, CARD_W, CARD_H);
    lv_obj_align(card, LV_ALIGN_TOP_LEFT, 10, SB_H + 10 + 2 * (CARD_H + CARD_GAP));
    screens_make_clickable_card(card, VIEW_EVENT_NAV_BLE);
    label = lv_label_create(card);
    lv_label_set_text(label, LV_SYMBOL_CHARGE " Calories");
    screens_set_label_style(label, view_font_label(), 0x8B949E);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 8, 0);
    s_act_cal = lv_label_create(card);
    lv_label_set_text(s_act_cal, "-- kcal");
    screens_set_label_style(s_act_cal, view_font_title(), 0xF0F6FC);
    lv_obj_align(s_act_cal, LV_ALIGN_RIGHT_MID, -8, 0);

    home_btn = screens_create_nav_button(s_activity, LV_SYMBOL_HOME, 0x58A6FF, VIEW_EVENT_NAV_HOME);
    lv_obj_align(home_btn, LV_ALIGN_TOP_LEFT, 248, 88);

    s_act_type = lv_label_create(s_activity);
    lv_label_set_text(s_act_type, "Type: --");
    screens_set_label_style(s_act_type, view_font_label(), 0x8B949E);
    lv_obj_align(s_act_type, LV_ALIGN_BOTTOM_LEFT, 10, -12);

    s_act_fall = lv_label_create(s_activity);
    lv_label_set_text(s_act_fall, "");
    screens_set_label_style(s_act_fall, view_font_label(), 0xFF4444);
    lv_obj_align(s_act_fall, LV_ALIGN_BOTTOM_RIGHT, -10, -12);
}
