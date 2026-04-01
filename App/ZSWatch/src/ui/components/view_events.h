/**
 * @file view_events.h
 * @brief IDs et types d'événements View -> Controller
 *
 * Contrat d'échange événementiel entre la tâche Vue et la ME principale.
 */

#ifndef VIEW_EVENTS_H
#define VIEW_EVENTS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VIEW_EVENT_NONE = 0,
    VIEW_EVENT_NAV_HOME,
    VIEW_EVENT_NAV_SETTINGS,
    VIEW_EVENT_NAV_NOTIFICATIONS,
    VIEW_EVENT_NAV_SENSORS,
    VIEW_EVENT_NAV_ACTIVITY,
    VIEW_EVENT_NAV_BLE,
    VIEW_EVENT_NAV_BACK,
    VIEW_EVENT_NAV_NEXT,
    VIEW_EVENT_BUTTON_PRESSED,
    VIEW_EVENT_BUTTON_LONG_PRESSED,
    VIEW_EVENT_BUTTON_SELECT,
    VIEW_EVENT_BUTTON_CONFIRM,
    VIEW_EVENT_GESTURE_SWIPE_LEFT,
    VIEW_EVENT_GESTURE_SWIPE_RIGHT,
    VIEW_EVENT_GESTURE_SWIPE_UP,
    VIEW_EVENT_GESTURE_SWIPE_DOWN,
    VIEW_EVENT_SETTING_CHANGED,
    VIEW_EVENT_BLE_TOGGLE,
    VIEW_EVENT_NOTIFICATION_DISMISS,
    VIEW_EVENT_USER_BASE = 0x100
} view_event_id_t;

typedef enum {
    VIEW_EVENT_SOURCE_NONE = 0,
    VIEW_EVENT_SOURCE_TOUCH,
    VIEW_EVENT_SOURCE_BUTTON,
    VIEW_EVENT_SOURCE_SYSTEM,
} view_event_source_t;

typedef struct {
    view_event_source_t source;
    uint8_t screen_id;
    int16_t x, y;
    int16_t target_left, target_top, target_right, target_bottom;
    int8_t button_id;
} view_event_data_t;

#ifdef __cplusplus
}
#endif

#endif /* VIEW_EVENTS_H */
