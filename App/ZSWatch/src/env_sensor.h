#ifndef ENV_SENSOR_H
#define ENV_SENSOR_H

#include <zephyr/drivers/sensor.h>

typedef struct {
    const struct device *hts221;
    const struct device *lps22hh;
    
    // Données stockées
    struct sensor_value temp_hts;
    struct sensor_value humidity;
    struct sensor_value pressure;
    struct sensor_value temp_lps;
} EnvSensor;

int env_init(EnvSensor *s);
int env_update(EnvSensor *s);

#endif