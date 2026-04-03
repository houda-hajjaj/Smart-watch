#include "../components/screens_internal.h"
#include "../components/styles.h"

void screen_home_create(void)
{
    lv_obj_t *dial, *ring, *row, *btn, *label;

    s_home = lv_obj_create(NULL);
    screens_apply_screen_base(s_home);
    screens_create_status_bar(s_home, SB_HOME);

    dial = lv_obj_create(s_home);
    lv_obj_remove_style_all(dial);
    lv_obj_set_size(dial, 140, 140);
    lv_obj_align(dial, LV_ALIGN_CENTER, 0, 14);
    lv_obj_remove_flag(dial, LV_OBJ_FLAG_SCROLLABLE);

    ring = lv_arc_create(dial);
    lv_obj_remove_style_all(ring);
    lv_obj_set_size(ring, 136, 136);
    lv_obj_center(ring);
    lv_arc_set_range(ring, 0, 100);
    lv_arc_set_value(ring, 100);
    lv_arc_set_bg_angles(ring, 0, 360);
    lv_obj_set_style_arc_color(ring, lv_color_hex(0x21262D), LV_PART_MAIN);
    lv_obj_set_style_arc_width(ring, 7, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ring, lv_color_hex(0x6ED4E8), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ring, 5, LV_PART_INDICATOR);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);

    row = lv_obj_create(dial);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, 140, 44);
    lv_obj_align(row, LV_ALIGN_CENTER, 0, -15);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 2, 0);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    s_home_hour = lv_label_create(row);
    lv_label_set_text(s_home_hour, "00");
    screens_set_label_style(s_home_hour, view_font_time(), 0xFFFFFF);
    label = lv_label_create(row);
    lv_label_set_text(label, ":");
    screens_set_label_style(label, view_font_time(), 0xFFFFFF);
    s_home_min = lv_label_create(row);
    lv_label_set_text(s_home_min, "00");
    screens_set_label_style(s_home_min, view_font_time(), 0xFFFFFF);
    label = lv_label_create(row);
    lv_label_set_text(label, ":");
    screens_set_label_style(label, view_font_time(), 0xFFFFFF);
    s_home_sec = lv_label_create(row);
    lv_label_set_text(s_home_sec, "00");
    screens_set_label_style(s_home_sec, view_font_time(), 0xFFFFFF);

    s_home_date = lv_label_create(dial);
    lv_label_set_text(s_home_date, "09/07/26");
    screens_set_label_style(s_home_date, view_font_label(), 0x8B949E);
    lv_obj_align(s_home_date, LV_ALIGN_CENTER, 0, 22);

    row = lv_obj_create(dial);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, 70, 16);
    lv_obj_align(row, LV_ALIGN_CENTER, 0, 40);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 5, 0);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    screens_create_step_icon(row);
    s_home_steps = lv_label_create(row);
    lv_label_set_text(s_home_steps, "0");
    screens_set_label_style(s_home_steps, view_font_title(), 0xF0F6FC);

    btn = screens_create_nav_button(s_home, LV_SYMBOL_IMAGE, 0x58A6FF, VIEW_EVENT_NAV_NOTIFICATIONS);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 14, 72);

    btn = screens_create_nav_button(s_home, LV_SYMBOL_WIFI, 0x58A6FF, VIEW_EVENT_NAV_BLE);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 14, 140);

    btn = screens_create_nav_button(s_home, LV_SYMBOL_GPS, 0x58A6FF, VIEW_EVENT_NAV_SENSORS);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -14, 72);

    btn = screens_create_nav_button(s_home, LV_SYMBOL_LOOP, 0x58A6FF, VIEW_EVENT_NAV_ACTIVITY);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -14, 140);
}

