#include "../components/screens_internal.h"
#include "../components/styles.h"

void screen_splash_create(void)
{
    lv_obj_t *btn, *label;

    s_splash = lv_obj_create(NULL);
    screens_apply_screen_base(s_splash);

    label = lv_label_create(s_splash);
    lv_label_set_text(label, "ZSWatch");
    screens_set_label_style(label, view_font_time(), 0x58A6FF);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -30);

    label = lv_label_create(s_splash);
    lv_label_set_text(label, "Smart Watch");
    screens_set_label_style(label, view_font_title(), 0x8B949E);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 15);

    btn = lv_obj_create(s_splash);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, 160, 40);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 65);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x238636), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn, 10, 0);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn, screens_nav_event_cb, LV_EVENT_CLICKED,
                        (void *)(uintptr_t)VIEW_EVENT_NAV_HOME);
    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_HOME " Continuer");
    screens_set_label_style(label, view_font_title(), 0xFFFFFF);
    lv_obj_center(label);
}

