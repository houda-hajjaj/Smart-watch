#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

#define BUTTON_EVENT_NONE        0U
#define BUTTON_EVENT_NEXT_SCENE  (1U << 0)
#define BUTTON_EVENT_WAKE        (1U << 1)

int buttons_init(void);
uint32_t buttons_consume_events(void);

#endif /* BUTTONS_H */
