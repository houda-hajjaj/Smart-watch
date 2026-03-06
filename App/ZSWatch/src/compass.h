#ifndef COMPASS_H
#define COMPASS_H

#include <zephyr/drivers/sensor.h>
#include <stdint.h>

typedef struct {
    double heading;         // angle en degrés (0-360)
    double x, y, z;         // dernières valeurs brutes (en µT)
    double offset_x, offset_y, offset_z; // calibration (optionnel)
} Compass;

void compass_init(Compass *comp);
void compass_update(Compass *comp, const struct sensor_value magn[3]);
double compass_get_heading(const Compass *comp);
const char* compass_direction_to_str(double heading);   // <-- ajout

#endif /* COMPASS_H */