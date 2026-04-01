#ifndef COMPASS_H
#define COMPASS_H

#include <zephyr/drivers/sensor.h>
#include <stdint.h>

typedef struct {
    double heading;         // angle filtré en degrés (0-360)
    double x, y, z;         // dernières valeurs compensées (en µT)
    double offset_x, offset_y, offset_z;
    double scale_x, scale_y, scale_z;
    double min_x, min_y, min_z;
    double max_x, max_y, max_z;
    double accel_x, accel_y, accel_z;
    double mag_x, mag_y, mag_z;
    bool initialized;
    bool heading_valid;
} Compass;

void compass_init(Compass *comp);
void compass_update(Compass *comp,
                    const struct sensor_value accel[3],
                    const struct sensor_value magn[3]);
double compass_get_heading(const Compass *comp);
const char* compass_direction_to_str(double heading);   // <-- ajout

#endif /* COMPASS_H */
