#ifndef POWER_THREAD_H
#define POWER_THREAD_H

#include <stdbool.h>

#include <zephyr/kernel.h>

void power_thread_start(void);
void power_thread_notify_activity(void);
bool power_thread_is_eco_mode(void);
bool power_thread_wait_until_awake(k_timeout_t timeout);

#endif /* POWER_THREAD_H */
