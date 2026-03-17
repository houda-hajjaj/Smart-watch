#ifndef PROCESSING_THREAD_H
#define PROCESSING_THREAD_H

/* User profile – adjust to actual user */
#define USER_WEIGHT_KG   70.0
#define USER_STRIDE_M    0.75

/**
 * @brief Initialise all processing modules (step counter, altimeter, etc.).
 *        Must be called before shared_data_timers_start().
 */
void processing_thread_init(void);

#endif /* PROCESSING_THREAD_H */
