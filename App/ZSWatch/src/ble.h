#ifndef BLE_H
#define BLE_H

#include <stddef.h>

/**
 * @brief Initialise la stack Bluetooth et démarre la publicité.
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
int ble_init(void);

/**
 * @brief Envoie une notification avec les données des capteurs.
 * @param data Pointeur vers les données à envoyer.
 * @param len  Longueur des données (en octets).
 *
 * Si aucun client n'est abonné, la notification n'est pas envoyée.
 */
void ble_notify_sensor_data(const uint8_t *data, size_t len);

#endif /* BLE_H */