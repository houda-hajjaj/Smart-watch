#ifndef ENV_SENSOR_H
#define ENV_SENSOR_H

#include <zephyr/drivers/sensor.h>

typedef struct {
    const struct device *stts751;
    
    // Données stockées
    struct sensor_value temp_hts;
    
} EnvSensor;

int env_init(EnvSensor *s);
int env_update(EnvSensor *s);

#endif