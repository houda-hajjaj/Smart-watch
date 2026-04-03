#include "../components/screens_internal.h"
#include "../components/styles.h"

void screen_ble_create(void)
{
    lv_obj_t *card, *label, *home_btn;

    s_ble = lv_obj_create(NULL);
    screens_apply_screen_base(s_ble);
    screens_create_status_bar(s_ble, SB_BLE);

    card = lv_obj_create(s_ble);
    lv_obj_add_style(card, view_style_card(), 0);
    lv_obj_set_size(card, 280, 96);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, SB_H + 18);
    screens_make_clickable_card(card, VIEW_EVENT_NAV_HOME);

    label = lv_label_create(card);
    lv_label_set_text(label, "Statut BLE");
    screens_set_label_style(label, view_font_label(), 0x8B949E);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 12, 6);

    s_ble_stat = lv_label_create(card);
    lv_label_set_text(s_ble_stat, "Deconnecte");
    screens_set_label_style(s_ble_stat, view_font_title(), 0xF0F6FC);
    lv_obj_align(s_ble_stat, LV_ALIGN_TOP_LEFT, 12, 26);

    label = lv_label_create(card);
    lv_label_set_text(label, "Bilan de liaison");
    screens_set_label_style(label, view_font_label(), 0x8B949E);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 12, 48);

    s_ble_rssi = lv_label_create(card);
    lv_label_set_text(s_ble_rssi, "RSSI: -- dBm");
    screens_set_label_style(s_ble_rssi, view_font_title(), 0xF0F6FC);
    lv_obj_align(s_ble_rssi, LV_ALIGN_TOP_LEFT, 12, 68);

    home_btn = screens_create_nav_button(s_ble, LV_SYMBOL_HOME, 0x58A6FF, VIEW_EVENT_NAV_HOME);
    lv_obj_set_size(home_btn, 48, 48);
    lv_obj_align(home_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
}
