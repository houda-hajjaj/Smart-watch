#ifndef MAG_SENSOR_H
#define MAG_SENSOR_H

#include <zephyr/drivers/sensor.h>

typedef struct {
    const struct device *dev;
    struct sensor_value magn[3];
} MagSensor;

int mag_init(MagSensor *s);
int mag_update(MagSensor *s);

#endif