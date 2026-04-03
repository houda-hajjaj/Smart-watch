#include "../components/screens_internal.h"
#include "../components/styles.h"

#define CMP_RING_SZ   130
#define CMP_CENTER_Y  117

void screen_compass_create(void)
{
    lv_obj_t *label, *home_btn;

    s_compass = lv_obj_create(NULL);
    screens_apply_screen_base(s_compass);
    screens_create_status_bar(s_compass, SB_COMPASS);

    s_cmp_ring = lv_arc_create(s_compass);
    lv_obj_remove_style_all(s_cmp_ring);
    lv_obj_set_size(s_cmp_ring, CMP_RING_SZ, CMP_RING_SZ);
    lv_obj_align(s_cmp_ring, LV_ALIGN_CENTER, 0, -3);
    lv_arc_set_range(s_cmp_ring, 0, 100);
    lv_arc_set_value(s_cmp_ring, 100);
    lv_arc_set_bg_angles(s_cmp_ring, 0, 360);
    lv_obj_set_style_arc_color(s_cmp_ring, lv_color_hex(0x21262D), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_cmp_ring, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_cmp_ring, lv_color_hex(0x6ED4E8), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(s_cmp_ring, 5, LV_PART_INDICATOR);
    screens_make_clickable(s_cmp_ring, VIEW_EVENT_NAV_ACTIVITY);

    label = lv_label_create(s_compass);
    lv_label_set_text(label, "N");
    screens_set_label_style(label, view_font_title(), 0xF85149);
    lv_obj_align_to(label, s_cmp_ring, LV_ALIGN_OUT_TOP_MID, 0, -4);

    label = lv_label_create(s_compass);
    lv_label_set_text(label, "S");
    screens_set_label_style(label, view_font_title(), 0x8B949E);
    lv_obj_align_to(label, s_cmp_ring, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

    label = lv_label_create(s_compass);
    lv_label_set_text(label, "E");
    screens_set_label_style(label, view_font_title(), 0x8B949E);
    lv_obj_align_to(label, s_cmp_ring, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    label = lv_label_create(s_compass);
    lv_label_set_text(label, "O");
    screens_set_label_style(label, view_font_title(), 0x8B949E);
    lv_obj_align_to(label, s_cmp_ring, LV_ALIGN_OUT_LEFT_MID, -6, 0);

    s_cmp_needle = lv_obj_create(s_compass);
    lv_obj_remove_style_all(s_cmp_needle);
    lv_obj_set_size(s_cmp_needle, 6, 45);
    lv_obj_set_pos(s_cmp_needle, 157, 72);
    lv_obj_set_style_bg_color(s_cmp_needle, lv_color_hex(0xF85149), 0);
    lv_obj_set_style_bg_opa(s_cmp_needle, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(s_cmp_needle, 3, 0);
    lv_obj_set_style_transform_pivot_x(s_cmp_needle, 3, 0);
    lv_obj_set_style_transform_pivot_y(s_cmp_needle, 45, 0);
    lv_obj_clear_flag(s_cmp_needle, LV_OBJ_FLAG_CLICKABLE);

    home_btn = screens_create_nav_button(s_compass, LV_SYMBOL_HOME, 0x58A6FF, VIEW_EVENT_NAV_HOME);
    lv_obj_set_size(home_btn, BTN_SM, BTN_SM);
    lv_obj_align(home_btn, LV_ALIGN_CENTER, 0, -3);

    s_cmp_heading = lv_label_create(s_compass);
    lv_label_set_text(s_cmp_heading, "0°");
    screens_set_label_style(s_cmp_heading, view_font_title(), 0xF0F6FC);
    lv_obj_align(s_cmp_heading, LV_ALIGN_BOTTOM_MID, -30, -10);

    s_cmp_dir = lv_label_create(s_compass);
    lv_label_set_text(s_cmp_dir, "N");
    screens_set_label_style(s_cmp_dir, view_font_title(), 0x58A6FF);
    lv_obj_align(s_cmp_dir, LV_ALIGN_BOTTOM_MID, 30, -10);
}
