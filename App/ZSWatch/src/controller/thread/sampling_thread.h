#ifndef SAMPLING_THREAD_H
#define SAMPLING_THREAD_H

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>

#include "ble/ble_service.h"

typedef struct {
    struct ble_sensor_data ble_data;
    float temperature_c;
    float humidity_pct;
    int pressure_hpa;
    float distance_km;
    float calories_kcal;
    float heading_deg;
    uint8_t activity;
    uint8_t weather_trend;
    float altitude_m;
    int floor_count;
    bool fall_detected;
    uint32_t sample_count;
    int64_t timestamp_ms;
    bool valid;
} SamplingData;

void sampling_thread_start(void);
bool sampling_thread_is_running(void);
const SamplingData *sampling_thread_get_latest(void);
bool sampling_thread_get_latest_copy(SamplingData *out);

#endif /* SAMPLING_THREAD_H */
