#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/drivers/sensor.h>

/**
 * @file ble_service.h
 * @brief Services BLE pour diffuser les données capteurs IKS01A3.
 *
 * Services standards (auto-décodés par nRF Connect) :
 *   - Environmental Sensing Service (0x181A)
 *       Temperature (0x2A6E)  – int16, 0.01 °C
 *       Humidity    (0x2A6F)  – uint16, 0.01 %
 *       Pressure    (0x2A6D)  – uint32, 0.1 Pa
 *
 * Service custom pour mouvement :
 *   UUID base : 12345678-1234-5678-1234-56789abc0000
 *       Accéléromètre (0x0004) – 3 × int16, mg
 *       Magnétomètre  (0x0005) – 3 × int16, mGauss
 *       Compteur pas  (0x0006) – uint32
 *       Horodatage RTC (0x0007) – chaîne UTF-8 YYYY-MM-DD HH:MM:SS
 *       Cap compas     (0x0008) – uint16, 0.1 degré
 *       Direction      (0x0009) – chaîne UTF-8 (N, NE, E, ...)
 */

#define BLE_RTC_TEXT_LEN 20
#define BLE_COMPASS_DIR_LEN 4

/* ── Données capteur empaquetées pour BLE ──────────────── */
struct ble_sensor_data {
    /* Environnement (format BLE SIG standard) */
    int16_t  temperature;   /* 0.01 °C   (ex: 2350 = 23.50°C) */
    uint16_t humidity;      /* 0.01 %    (ex: 6520 = 65.20%)   */
    uint32_t pressure;      /* 0.1 Pa    (ex: 1013250 = 101325.0 Pa = 1013.25 hPa) */

    /* Mouvement */
    int16_t  accel[3];      /* mg         */
    int16_t  magn[3];       /* mGauss     */

    /* Pédomètre */
    uint32_t steps;

    /* RTC */
    char rtc_text[BLE_RTC_TEXT_LEN];

    /* Compas */
    uint16_t heading_tenths_deg;
    char compass_direction[BLE_COMPASS_DIR_LEN];
};

/**
 * @brief Initialise le stack BLE et enregistre les services GATT.
 * @return 0 en cas de succès, code d'erreur sinon.
 */
int ble_service_init(void);

/**
 * @brief Met à jour les données internes et envoie des notifications
 *        aux clients BLE connectés.
 * @param data  Structure contenant les dernières mesures capteur.
 */
void ble_service_update(const struct ble_sensor_data *data);

/**
 * @brief Indique si un client central est connecté.
 * @return true si connecté.
 */
bool ble_service_is_connected(void);

/**
 * @brief Read the current RSSI for the active BLE connection.
 * @param out_rssi_dbm Destination for the RSSI in dBm.
 * @return true if an RSSI value was read, false otherwise.
 */
bool ble_service_get_rssi_dbm(int8_t *out_rssi_dbm);

#endif /* BLE_SERVICE_H */
