#include "motion_sensor.h"
#include <zephyr/device.h>
#include <stdio.h>

int motion_init(MotionSensor *s) {
    s->dev = DEVICE_DT_GET_ONE(st_lsm6dso);
    if (!device_is_ready(s->dev)) return -1;
    
    // Configuration optionnelle (Fréquence à 208Hz comme dans l'original)
    struct sensor_value odr = { .val1 = 208, .val2 = 0 };
    sensor_attr_set(s->dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr);
    return 0;
}

int motion_update(MotionSensor *s) {
    if (sensor_sample_fetch(s->dev) < 0) return -1;
    sensor_channel_get(s->dev, SENSOR_CHAN_ACCEL_XYZ, s->accel);
    sensor_channel_get(s->dev, SENSOR_CHAN_GYRO_XYZ, s->gyro);
    return 0;
}