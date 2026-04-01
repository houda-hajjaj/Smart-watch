/**
 * @file styles.c
 * @brief Styles graphiques de la Vue (IHM seulement)
 *
 * Style cours MVC IoT : ce module ne contient aucune logique de contrôle,
 * uniquement la configuration visuelle LVGL (styles, couleurs, polices).
 */

#include "styles.h"

static bool s_initialized;
static lv_style_t s_style_screen;
static lv_style_t s_style_card;
static lv_style_t s_style_card_pressed;
static lv_style_t s_style_back_button;

void view_styles_init(void)
{
    if (s_initialized) {
        return;
    }

    lv_style_init(&s_style_screen);
    lv_style_set_bg_color(&s_style_screen, lv_color_hex(VIEW_UI_BG_DARK));
    lv_style_set_bg_opa(&s_style_screen, LV_OPA_COVER);
    lv_style_set_border_width(&s_style_screen, 0);
    lv_style_set_pad_all(&s_style_screen, 0);

    lv_style_init(&s_style_card);
    lv_style_set_bg_color(&s_style_card, lv_color_hex(VIEW_UI_BG_CARD));
    lv_style_set_bg_opa(&s_style_card, LV_OPA_COVER);
    lv_style_set_radius(&s_style_card, 12);
    lv_style_set_border_width(&s_style_card, 0);
    lv_style_set_pad_all(&s_style_card, 8);
    lv_style_set_shadow_width(&s_style_card, 0);

    lv_style_init(&s_style_card_pressed);
    lv_style_set_bg_color(&s_style_card_pressed, lv_color_hex(VIEW_UI_BG_CARD_ALT));
    lv_style_set_bg_opa(&s_style_card_pressed, LV_OPA_COVER);

    lv_style_init(&s_style_back_button);
    lv_style_set_bg_color(&s_style_back_button, lv_color_hex(VIEW_UI_BG_CARD));
    lv_style_set_bg_opa(&s_style_back_button, LV_OPA_COVER);
    lv_style_set_radius(&s_style_back_button, 10);
    lv_style_set_border_width(&s_style_back_button, 0);
    lv_style_set_shadow_width(&s_style_back_button, 0);

    s_initialized = true;
}

const lv_style_t *view_style_screen(void) { return &s_style_screen; }
const lv_style_t *view_style_card(void) { return &s_style_card; }
const lv_style_t *view_style_card_pressed(void) { return &s_style_card_pressed; }
const lv_style_t *view_style_back_button(void) { return &s_style_back_button; }

const lv_font_t *view_font_time(void) { return &lv_font_montserrat_14; }
const lv_font_t *view_font_title(void) { return &lv_font_montserrat_14; }
const lv_font_t *view_font_value(void) { return &lv_font_montserrat_14; }
const lv_font_t *view_font_label(void) { return &lv_font_montserrat_14; }
