/**
 * @file me_vue.c
 * @brief Machine à états de la Vue : splash puis home, file d'écran différé, transitions LVGL.
 */

#include "me_vue.h"
#include "screens.h"
#include <lvgl.h>
#include <string.h>

static view_config_t s_config;
static void *s_display;

static struct {
    view_screen_id_t current_screen;
    view_screen_id_t pending_screen;
    uint32_t pending_delay_ms;
    view_transition_t pending_transition;
    bool initialized;
} s_view;

#define VIEW_TRANSITION_SPEED_MS 300
#define VIEW_SPLASH_DURATION_MS 5000
#define VIEW_BOOT_SCREEN VIEW_SCREEN_SPLASH

static void default_view_event_cb(uint32_t event_id,
                                  const view_event_data_t *event_data,
                                  void *user_data);

/* Charge l'écran cible avec animation ou sans (instantané). */
static void apply_transition(lv_obj_t *target, view_transition_t transition)
{
    if (!target) return;

    switch (transition) {
    case VIEW_TRANSITION_FADE:
        lv_screen_load_anim(target, LV_SCR_LOAD_ANIM_FADE_ON,
                            VIEW_TRANSITION_SPEED_MS, 0, false);
        break;
    case VIEW_TRANSITION_SLIDE_LEFT:
        lv_screen_load_anim(target, LV_SCR_LOAD_ANIM_MOVE_LEFT,
                            VIEW_TRANSITION_SPEED_MS, 0, false);
        break;
    case VIEW_TRANSITION_SLIDE_RIGHT:
        lv_screen_load_anim(target, LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                            VIEW_TRANSITION_SPEED_MS, 0, false);
        break;
    case VIEW_TRANSITION_SLIDE_UP:
        lv_screen_load_anim(target, LV_SCR_LOAD_ANIM_MOVE_TOP,
                            VIEW_TRANSITION_SPEED_MS, 0, false);
        break;
    case VIEW_TRANSITION_SLIDE_DOWN:
        lv_screen_load_anim(target, LV_SCR_LOAD_ANIM_MOVE_BOTTOM,
                            VIEW_TRANSITION_SPEED_MS, 0, false);
        break;
    case VIEW_TRANSITION_NONE:
    default:
        lv_screen_load(target);
        break;
    }
}

/* Si un changement d'écran était planifié avec délai, l'exécute quand l'heure est venue. */
static void process_pending_transition(uint32_t now_ms)
{
    lv_obj_t *target;

    if (s_view.pending_screen == VIEW_SCREEN_NONE) return;
    if (now_ms < s_view.pending_delay_ms) return;

    target = view_screens_get(s_view.pending_screen);
    if (!target) {
        s_view.pending_screen = VIEW_SCREEN_NONE;
        return;
    }

    apply_transition(target, s_view.pending_transition);
    s_view.current_screen = s_view.pending_screen;
    s_view.pending_screen = VIEW_SCREEN_NONE;
}

/* Construit les écrans, affiche le splash ; passage home automatique après VIEW_SPLASH_DURATION_MS. */
int ME_VUE_init(const view_config_t *config)
{
    lv_obj_t *initial_screen;

    if (s_view.initialized) return 0;

    memset(&s_view, 0, sizeof(s_view));
    if (config) {
        s_config = *config;
    } else {
        s_config.event_callback = default_view_event_cb;
        s_config.event_user_data = NULL;
    }

    s_display = lv_display_get_default();
    if (!s_display) return -1;

    if (view_screens_init(&s_config) != 0) return -1;

    s_view.current_screen = VIEW_BOOT_SCREEN;
    s_view.pending_screen = VIEW_SCREEN_HOME;
    s_view.pending_transition = VIEW_TRANSITION_NONE;
    s_view.pending_delay_ms = (uint32_t)lv_tick_get() + VIEW_SPLASH_DURATION_MS;
    s_view.initialized = true;

    initial_screen = view_screens_get(VIEW_BOOT_SCREEN);
    if (!initial_screen) return -1;

    lv_screen_load(initial_screen);
    return 0;
}

/* Détruit les objets LVGL des écrans. */
void ME_VUE_deinit(void)
{
    if (!s_view.initialized) return;
    view_screens_deinit();
    s_view.initialized = false;
}

/* À chaque tick : applique les changements d'écran en attente. */
void ME_VUE_process(void)
{
    if (!s_view.initialized) return;
    process_pending_transition((uint32_t)lv_tick_get());
}

/* Rafraîchit les labels depuis le modèle (voir view_screens_update). */
void ME_VUE_update(const view_model_data_t *model)
{
    if (!s_view.initialized || !model) return;
    view_screens_update(model);
}

/* Remonte un événement sans détail (wrapper). */
void ME_VUE_send_event(uint32_t event_id)
{
    ME_VUE_send_event_data(event_id, NULL);
}

/* Appelle le callback configuré (même chemin que screens_send_event côté LVGL). */
void ME_VUE_send_event_data(uint32_t event_id,
                                         const view_event_data_t *event_data)
{
    if (s_config.event_callback) {
        s_config.event_callback(event_id, event_data, s_config.event_user_data);
    }
}

/* Enchaîne les écrans dans l'ordre défini (bouton « suivant » / swipe gauche). */
void ME_VUE_show_next_screen(void)
{
    view_screen_id_t next = VIEW_SCREEN_HOME;
    switch (s_view.current_screen) {
    case VIEW_SCREEN_SPLASH: next = VIEW_SCREEN_HOME; break;
    case VIEW_SCREEN_HOME: next = VIEW_SCREEN_NOTIFICATIONS; break;
    case VIEW_SCREEN_NOTIFICATIONS: next = VIEW_SCREEN_SENSORS; break;
    case VIEW_SCREEN_SENSORS: next = VIEW_SCREEN_ACTIVITY; break;
    case VIEW_SCREEN_ACTIVITY: next = VIEW_SCREEN_BLE; break;
    case VIEW_SCREEN_BLE:
    default: next = VIEW_SCREEN_HOME; break;
    }
    ME_VUE_show_screen(next, VIEW_TRANSITION_SLIDE_LEFT, 0);
}

/* Programme un écran (immédiat si delay_ms == 0). */
void ME_VUE_show_screen(view_screen_id_t screen_id,
                                     view_transition_t transition,
                                     uint32_t delay_ms)
{
    if (!s_view.initialized) return;
    if (screen_id <= VIEW_SCREEN_NONE || screen_id >= VIEW_SCREEN_COUNT) return;

    uint32_t now = (uint32_t)lv_tick_get();
    s_view.pending_screen = screen_id;
    s_view.pending_transition = transition;
    s_view.pending_delay_ms = now + delay_ms;

    if (delay_ms == 0U) process_pending_transition(now);
}

view_screen_id_t ME_VUE_get_current_screen(void)
{
    return s_view.current_screen;
}

void *ME_VUE_get_display(void)
{
    return s_display;
}

/* Ordre circulaire inverse pour swipe droite / retour logique. */
static void show_previous_screen(void)
{
    view_screen_id_t prev = VIEW_SCREEN_HOME;

    switch (s_view.current_screen) {
    case VIEW_SCREEN_SPLASH:
    case VIEW_SCREEN_HOME:
        prev = VIEW_SCREEN_BLE;
        break;
    case VIEW_SCREEN_NOTIFICATIONS:
        prev = VIEW_SCREEN_HOME;
        break;
    case VIEW_SCREEN_SENSORS:
        prev = VIEW_SCREEN_NOTIFICATIONS;
        break;
    case VIEW_SCREEN_ACTIVITY:
        prev = VIEW_SCREEN_SENSORS;
        break;
    case VIEW_SCREEN_BLE:
        prev = VIEW_SCREEN_ACTIVITY;
        break;
    case VIEW_SCREEN_SETTINGS:
    default:
        prev = VIEW_SCREEN_HOME;
        break;
    }

    ME_VUE_show_screen(prev, VIEW_TRANSITION_SLIDE_RIGHT, 0);
}

/* Navigation par défaut si l'appli ne fournit pas de callback personnalisé. */
static void default_view_event_cb(uint32_t event_id,
                                  const view_event_data_t *event_data,
                                  void *user_data)
{
    ARG_UNUSED(event_data);
    ARG_UNUSED(user_data);

    switch ((view_event_id_t)event_id) {
    case VIEW_EVENT_NAV_HOME:
        ME_VUE_show_screen(VIEW_SCREEN_HOME, VIEW_TRANSITION_FADE, 0);
        break;
    case VIEW_EVENT_NAV_NOTIFICATIONS:
        ME_VUE_show_screen(VIEW_SCREEN_NOTIFICATIONS, VIEW_TRANSITION_SLIDE_LEFT, 0);
        break;
    case VIEW_EVENT_NAV_SENSORS:
        ME_VUE_show_screen(VIEW_SCREEN_SENSORS, VIEW_TRANSITION_SLIDE_LEFT, 0);
        break;
    case VIEW_EVENT_NAV_ACTIVITY:
        ME_VUE_show_screen(VIEW_SCREEN_ACTIVITY, VIEW_TRANSITION_SLIDE_LEFT, 0);
        break;
    case VIEW_EVENT_NAV_BLE:
        ME_VUE_show_screen(VIEW_SCREEN_BLE, VIEW_TRANSITION_SLIDE_LEFT, 0);
        break;
    case VIEW_EVENT_NAV_NEXT:
    case VIEW_EVENT_GESTURE_SWIPE_LEFT:
        ME_VUE_show_next_screen();
        break;
    case VIEW_EVENT_GESTURE_SWIPE_RIGHT:
        show_previous_screen();
        break;
    default:
        break;
    }
}
