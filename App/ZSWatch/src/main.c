#include <zephyr/kernel.h>
#include <stdio.h>
#include "motion_sensor.h"
#include "mag_sensor.h"
#include "env_sensor.h"
#include "step_counter.h"
#include "ble_service.h"
#include "compass.h"
#include "altimeter.h"
#include "fusion.h"   
#include "activity.h"
#include "weather.h"

int main(void) {
    MotionSensor imu;
    MagSensor mag;
    EnvSensor env;
    StepCounter steps;
    Compass comp;
    Altimeter alt;

    compass_init(&comp);
    altimeter_init(&alt);
    fusion_init();   
    weather_init();

    // Initialisation des capteurs
    if (motion_init(&imu) != 0 || mag_init(&mag) != 0 || env_init(&env) != 0) {
        printf("Erreur d'initialisation des capteurs.\n");
        return -1;
    }

    // Initialisation du compteur de pas
    step_counter_init(&steps);

    // Initialisation BLE
    if (ble_service_init() != 0) {
        printf("Erreur d'initialisation BLE.\n");
        // Continue quand même sans BLE
    }

    while (1) {
        printf("\033[H\033[J"); // Rafraîchit la console
        printf("=== IKS01A3 DASHBOARD FULL ===\n\n");

        // Mise à jour des capteurs
        env_update(&env);
        motion_update(&imu);
        mag_update(&mag);

        // Mise à jour de l'altimètre
        altimeter_update(&alt, sensor_value_to_double(&env.pressure));

        // --- Affichage console ---
        printf("HTS221 : Temp: %.1f C | Hum: %.1f%%\n",
               sensor_value_to_double(&env.temp_hts),
               sensor_value_to_double(&env.humidity));
        printf("LPS22HH: Press: %.3f kPa | Temp: %.1f C\n",
               sensor_value_to_double(&env.pressure),
               sensor_value_to_double(&env.temp_lps));
        printf("Altitude: %.1f m\n\n", altimeter_get_altitude(&alt));

        printf("LSM6DSO: Accel X: %.2f Y: %.2f Z: %.2f\n",
               sensor_value_to_double(&imu.accel[0]),
               sensor_value_to_double(&imu.accel[1]),
               sensor_value_to_double(&imu.accel[2]));

        printf("LIS2MDL: Magn  X: %.3f Y: %.3f Z: %.3f\n",
               sensor_value_to_double(&mag.magn[0]),
               sensor_value_to_double(&mag.magn[1]),
               sensor_value_to_double(&mag.magn[2]));

        compass_update(&comp, mag.magn);
        printf("Boussole: %.1f°\n", compass_get_heading(&comp));

       
        static uint32_t last_time = 0;
        uint32_t now = k_uptime_get_32();
        uint32_t delta = now - last_time;
        if (last_time != 0 && delta > 0) {
            Orientation orient = fusion_update(imu.accel, imu.gyro, mag.magn, delta);
            printf("Orientation: Roulis=%.1f°, Tangage=%.1f°, Lacet=%.1f°\n",
                   orient.roll, orient.pitch, orient.yaw);
        }
        last_time = now;
        

        activity_t act = activity_update(imu.accel);
        printf("Activité: %s\n", activity_to_str(act));

        weather_trend_t trend = weather_update(sensor_value_to_double(&env.pressure));
        printf("Tendance pression: %s\n", weather_trend_to_str(trend));

        // Mise à jour du compteur de pas
        step_counter_update(&steps, imu.accel);
        printf("\nPedometre: %u pas (mag: %.2f g)\n",
               step_counter_get_steps(&steps), steps.last_magnitude);

        // ── Envoi BLE ──
        struct ble_sensor_data ble_data = {
            .temperature = (int16_t)(sensor_value_to_double(&env.temp_hts) * 100),
            .humidity    = (uint16_t)(sensor_value_to_double(&env.humidity) * 100),
            .pressure    = (uint32_t)(sensor_value_to_double(&env.pressure) * 10000),
            .accel = {
                (int16_t)(sensor_value_to_double(&imu.accel[0]) * 1000),
                (int16_t)(sensor_value_to_double(&imu.accel[1]) * 1000),
                (int16_t)(sensor_value_to_double(&imu.accel[2]) * 1000),
            },
            .magn = {
                (int16_t)(sensor_value_to_double(&mag.magn[0]) * 1000),
                (int16_t)(sensor_value_to_double(&mag.magn[1]) * 1000),
                (int16_t)(sensor_value_to_double(&mag.magn[2]) * 1000),
            },
            .steps = step_counter_get_steps(&steps),
        };
        ble_service_update(&ble_data);

        printf("BLE: %s\n",
               ble_service_is_connected() ? "Connecte" : "En attente...");

        k_sleep(K_MSEC(200));
    }
    return 0;
}