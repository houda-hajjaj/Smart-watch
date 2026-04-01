#ifndef DATA_INIT_THREAD_H
#define DATA_INIT_THREAD_H

#include <stdbool.h>

#include <zephyr/kernel.h>

#include "sensors/motion_sensor.h"
#include "sensors/env_sensor.h"
#include "sensors/mag_sensor.h"
#include "rtc/rtc.h"

typedef struct {
    MotionSensor motion;
    EnvSensor env;
    MagSensor mag;
    WatchRTC rtc;

    bool motion_ready;
    bool env_ready;
    bool mag_ready;
    bool rtc_ready;
    bool ble_ready;
    bool initialized;
} DataInitContext;

void data_init_thread_start(void);
bool data_init_thread_wait_done(k_timeout_t timeout);
bool data_init_thread_wait_ready(k_timeout_t timeout);
bool data_init_thread_is_ready(void);
DataInitContext *data_init_thread_get_context_mutable(void);
const DataInitContext *data_init_thread_get_context(void);

#endif /* DATA_INIT_THREAD_H */
