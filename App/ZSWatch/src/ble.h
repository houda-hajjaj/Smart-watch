#ifndef BLE_H
#define BLE_H

#include <zephyr/types.h>

/**
 * @brief Initialise le BLE, démarre la publicité.
 */
void ble_init(void);

/**
 * @brief Met à jour la température (valeur en 0.01 °C)
 * @param temp_100 temperature * 100 (ex: 25.0°C -> 2500)
 */
void ble_update_temperature(int16_t temp_100);

/**
 * @brief Met à jour l'humidité (valeur en 0.01 %)
 * @param humi_100 humidité * 100 (ex: 50.0% -> 5000)
 */
void ble_update_humidity(uint16_t humi_100);

/**
 * @brief Met à jour la pression (valeur en Pa, ou 0.01 kPa)
 *        À adapter selon le format choisi.
 * @param pressure pression en pascals (ex: 101325)
 */
void ble_update_pressure(uint32_t pressure);

/**
 * @brief Met à jour l'accélération (axes en 0.01 m/s² ou 0.01 G)
 * @param x_100, y_100, z_100 valeurs * 100
 */
void ble_update_acceleration(int16_t x_100, int16_t y_100, int16_t z_100);

/**
 * @brief Met à jour le champ magnétique (axes en 0.01 µT)
 * @param x_100, y_100, z_100 valeurs * 100
 */
void ble_update_magnetometer(int16_t x_100, int16_t y_100, int16_t z_100);

#endif /* BLE_H */