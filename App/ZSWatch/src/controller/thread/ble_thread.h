#ifndef BLE_THREAD_H
#define BLE_THREAD_H

#include <stdint.h>

#define BLE_THREAD_UPDATE_PERIOD_MS 50

void ble_thread_start(void);
void ble_thread_notify_sample(uint32_t sample_count);

#endif /* BLE_THREAD_H */
