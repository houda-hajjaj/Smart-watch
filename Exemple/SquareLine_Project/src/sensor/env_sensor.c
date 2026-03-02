#include "env_sensor.h"
#include <zephyr/device.h>

int env_init(EnvSensor *s) {
    // Récupération des instances depuis le Device Tree
    s->stts751 = DEVICE_DT_GET_ONE(st_stts751);

    if (!device_is_ready(s->stts751)) {
        return -1;
    }

    struct sensor_value temp;
    if (sensor_sample_fetch(s->stts751) < 0 || sensor_channel_get(s->stts751, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
        return -1;
    }

    s->temp_hts = temp;

    return 0;
}

int env_update(EnvSensor *s) {
    // Lecture des échantillons (Fetch)
    if (sensor_sample_fetch(s->stts751) < 0) {
        return -1;
    }

    // Extraction des données (Get)
    sensor_channel_get(s->stts751, SENSOR_CHAN_AMBIENT_TEMP, &s->temp_hts);

    return 0;
}