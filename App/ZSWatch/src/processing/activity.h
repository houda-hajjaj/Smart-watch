#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <zephyr/drivers/sensor.h>  // pour struct sensor_value

typedef enum {
    ACTIVITY_UNKNOWN,
    ACTIVITY_REST,
    ACTIVITY_WALKING,
    ACTIVITY_RUNNING,
    // ajoutez d'autres états si besoin
} activity_t;

/**
 * @brief Met à jour la détection d'activité à partir des accélérations.
 * @param accel Tableau de 3 sensor_value (X, Y, Z) en m/s².
 * @return activity_t l'activité estimée.
 */
activity_t activity_update(const struct sensor_value accel[3]);

/**
 * @brief Convertit une activité en chaîne lisible.
 */
const char* activity_to_str(activity_t act);

#endif /* ACTIVITY_H */