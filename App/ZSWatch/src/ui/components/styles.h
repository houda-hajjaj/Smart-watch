/**
 * @file styles.h
 * @brief API des styles LVGL pour la couche Vue
 */

#ifndef ZSWATCH_VIEW_STYLES_H
#define ZSWATCH_VIEW_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VIEW_UI_BG_DARK        0x0D1117
#define VIEW_UI_BG_CARD        0x161B22
#define VIEW_UI_BG_CARD_ALT    0x21262D
#define VIEW_UI_ACCENT         0x58A6FF
#define VIEW_UI_ACCENT_GREEN   0x3FB950
#define VIEW_UI_TEXT_PRIMARY   0xF0F6FC
#define VIEW_UI_TEXT_SECONDARY 0x8B949E

void view_styles_init(void);

const lv_style_t *view_style_screen(void);
const lv_style_t *view_style_card(void);
const lv_style_t *view_style_card_pressed(void);
const lv_style_t *view_style_back_button(void);

const lv_font_t *view_font_time(void);
const lv_font_t *view_font_title(void);
const lv_font_t *view_font_value(void);
const lv_font_t *view_font_label(void);

#ifdef __cplusplus
}
#endif

#endif /* ZSWATCH_VIEW_STYLES_H */
