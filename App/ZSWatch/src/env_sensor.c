#include "env_sensor.h"
#include <zephyr/device.h>

int env_init(EnvSensor *s) {
    // Récupération des instances depuis le Device Tree
    s->hts221 = DEVICE_DT_GET_ONE(st_hts221);
    s->lps22hh = DEVICE_DT_GET_ONE(st_lps22hh);

    if (!device_is_ready(s->hts221) || !device_is_ready(s->lps22hh)) {
        return -1;
    }

    // Configuration de la fréquence du LPS22HH à 100 Hz
    struct sensor_value odr = { .val1 = 100, .val2 = 0 };
    sensor_attr_set(s->lps22hh, SENSOR_CHAN_ALL, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr);

    return 0;
}

int env_update(EnvSensor *s) {
    // Lecture des échantillons (Fetch)
    if (sensor_sample_fetch(s->hts221) < 0 || sensor_sample_fetch(s->lps22hh) < 0) {
        return -1;
    }

    // Extraction des données (Get)
    sensor_channel_get(s->hts221, SENSOR_CHAN_AMBIENT_TEMP, &s->temp_hts);
    sensor_channel_get(s->hts221, SENSOR_CHAN_HUMIDITY, &s->humidity);
    sensor_channel_get(s->lps22hh, SENSOR_CHAN_PRESS, &s->pressure);
    sensor_channel_get(s->lps22hh, SENSOR_CHAN_AMBIENT_TEMP, &s->temp_lps);

    return 0;
}