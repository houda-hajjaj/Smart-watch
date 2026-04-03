#ifndef FUSION_SIMPLE_H
#define FUSION_SIMPLE_H

#include <zephyr/drivers/sensor.h>

typedef struct {
    double roll;   // degrés
    double pitch;
    double yaw;
} Orientation;

/**
 * @brief Initialise le filtre de fusion.
 */
void fusion_init(void);

/**
 * @brief Met à jour l'orientation avec les nouvelles mesures.
 * @param accel Tableau de 3 sensor_value (m/s²)
 * @param gyro  Tableau de 3 sensor_value (rad/s)
 * @param magn  Tableau de 3 sensor_value (µT)
 * @param delta_ms Temps écoulé depuis la dernière mise à jour (ms)
 * @return Orientation actuelle.
 */
Orientation fusion_update(const struct sensor_value accel[3],
                          const struct sensor_value gyro[3],
                          const struct sensor_value magn[3],
                          uint32_t delta_ms);

#endif /* FUSION_SIMPLE_H */