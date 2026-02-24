#ifndef COMPASS_H
#define COMPASS_H

#include <zephyr/drivers/sensor.h>
#include <stdint.h>

typedef struct {
    double heading;         // angle en degrés (0-360)
    double x, y, z;         // dernières valeurs brutes (en µT)
    double offset_x, offset_y, offset_z; // calibration (optionnel)
} Compass;

/**
 * @brief Initialise la boussole (calibration éventuelle).
 * @param comp Pointeur vers la structure Compass.
 */
void compass_init(Compass *comp);

/**
 * @brief Met à jour la boussole avec les nouvelles données magnétiques.
 * @param comp Pointeur vers la structure Compass.
 * @param magn Tableau de 3 sensor_value (X, Y, Z) en µT.
 */
void compass_update(Compass *comp, const struct sensor_value magn[3]);

/**
 * @brief Retourne l'angle de la boussole (degrés).
 * @param comp Pointeur vers la structure Compass.
 * @return Angle en degrés (0-360).
 */
double compass_get_heading(const Compass *comp);

#endif /* COMPASS_H */