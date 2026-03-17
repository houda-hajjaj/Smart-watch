#ifndef SENSOR_THREAD_H
#define SENSOR_THREAD_H

/**
 * @brief Initialise all hardware sensors (motion, mag, env, RTC).
 *        Must be called before shared_data_timers_start().
 * @return 0 on success, negative errno on failure.
 */
int sensor_thread_init(void);

#endif /* SENSOR_THREAD_H */
