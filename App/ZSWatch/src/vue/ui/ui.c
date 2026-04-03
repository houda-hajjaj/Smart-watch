/**
 * @file ui.c
 * @brief Implémentation de ui.h : tout passe par ME_VUE (écrans, événements, modèle).
 */

#include "ui.h"

#include <lvgl.h>

/* Initialise l'IHM ; @p config peut fournir le callback vers le contrôleur. */
int ui_init_with_config(const view_config_t *config)
{
    return ME_VUE_init(config);
}

/* Même chose sans config (callback interne par défaut dans ME_VUE). */
void ui_init(void)
{
    (void)ME_VUE_init((const view_config_t *)0);
}

/* Libère les écrans LVGL créés par la Vue. */
void ui_destroy(void)
{
    ME_VUE_deinit();
}

/* À appeler régulièrement : LVGL + transitions / timers côté Vue. */
void ui_process(void)
{
    (void)lv_timer_handler();
    ME_VUE_process();
}

/* Met à jour les textes / widgets à partir du modèle (données capteurs, heure, etc.). */
void ui_update(const view_model_data_t *model)
{
    ME_VUE_update(model);
}
