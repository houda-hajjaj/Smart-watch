#include "mag_sensor.h"
#include <zephyr/device.h>

int mag_init(MagSensor *s) {
    s->dev = DEVICE_DT_GET_ONE(st_lis2mdl);
    if (!device_is_ready(s->dev)) return -1;

    struct sensor_value odr = { .val1 = 100, .val2 = 0 };
    sensor_attr_set(s->dev, SENSOR_CHAN_ALL, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr);
    return 0;
}

int mag_update(MagSensor *s) {
    if (sensor_sample_fetch(s->dev) < 0) return -1;
    sensor_channel_get(s->dev, SENSOR_CHAN_MAGN_XYZ, s->magn);
    return 0;
}