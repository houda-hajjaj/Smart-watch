#ifndef SLEEP_THREAD_H
#define SLEEP_THREAD_H

#include <stdbool.h>

/**
 * @brief Returns true while the system is in sleep (low-power) mode.
 */
bool sleep_is_active(void);

#endif /* SLEEP_THREAD_H */
