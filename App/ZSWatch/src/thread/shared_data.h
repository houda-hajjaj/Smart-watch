#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/rtc.h>
#include <stdbool.h>

#include "processing/activity.h"
#include "processing/fusion.h"
#include "processing/weather.h"
#include "ble/ble_service.h"


struct watch_raw_data {
    struct sensor_value accel[3];   /* LSM6DSO  – m/s²   */
    struct sensor_value gyro[3];    /* LSM6DSO  – rad/s  */
    struct sensor_value magn[3];    /* LIS2MDL  – µT     */
    double   temperature_c;         /* HTS221   – °C     */
    double   humidity_pct;          /* HTS221   – %      */
    double   pressure_kpa;          /* LPS22HH  – kPa    */
    struct   rtc_time time;         /* RV-8263-C8        */
    uint32_t timestamp_ms;          /* k_uptime_get_32() */
};


struct watch_processed_data {
    uint32_t        steps;
    double          distance_m;
    int             floors;
    double          calories;
    activity_t      activity;
    double          heading;        
    double          altitude_m;
    Orientation     orientation;    
    bool            fall_detected;
    weather_trend_t weather;
    struct ble_sensor_data ble_data; 
};


extern struct watch_raw_data       g_raw;
extern struct watch_processed_data g_processed;

/* ════════════════════════════════════════════════════════════════
 *  Semaphores
 *
 *  timer1 (100 ms) ──► sem_sample_tick  ──► sensor_thread
 *  sensor_thread   ──► sem_sensor_ready ──► processing_thread
 *  processing_thread──► sem_ble_notify  ──► ble_thread
 *  timer0 (30 s)   ──► sem_sleep        ──► sleep_thread
 * ════════════════════════════════════════════════════════════════ */
extern struct k_sem sem_sample_tick;    /* 100 ms tick → sensor_thread      */
extern struct k_sem sem_sensor_ready;   /* sensor done → processing_thread  */
extern struct k_sem sem_ble_notify;     /* processing done → ble_thread     */
extern struct k_sem sem_sleep;          /* 30 s idle → sleep_thread         */


extern struct k_mutex mutex_data;

/** Initialise shared structs to zero. Must be called before any thread runs. */
void shared_data_init(void);

/** Start timer0 (30 s sleep) and timer1 (100 ms sample).
 *  Call only after all sensor / module inits are complete. */
void shared_data_timers_start(void);

/** Reset the sleep timer (call on any user activity / motion event). */
void shared_data_sleep_timer_reset(void);

#endif /* SHARED_DATA_H */
