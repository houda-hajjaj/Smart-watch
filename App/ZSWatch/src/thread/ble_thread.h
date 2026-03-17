#ifndef BLE_THREAD_H
#define BLE_THREAD_H

/**
 * @brief Initialise the BLE stack and GATT services.
 *        Must be called before shared_data_timers_start().
 * @return 0 on success, negative errno on failure.
 */
int ble_thread_init(void);

#endif /* BLE_THREAD_H */
