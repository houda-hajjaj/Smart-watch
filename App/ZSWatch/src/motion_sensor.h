            #ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <zephyr/drivers/sensor.h>

typedef struct {
    const struct device *dev;
    struct sensor_value accel[3];
    struct sensor_value gyro[3];
} MotionSensor;

int motion_init(MotionSensor *s);
int motion_update(MotionSensor *s);

#endif