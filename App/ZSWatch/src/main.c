#include <zephyr/kernel.h>
#include <stdio.h>
#include "motion_sensor.h"
#include "mag_sensor.h"
#include "env_sensor.h"
#include "ble.h"

int main(void) {
    MotionSensor imu;
    MagSensor mag;
    EnvSensor env;

    // Initialisation des capteurs
    if (motion_init(&imu) != 0 || mag_init(&mag) != 0 || env_init(&env) != 0) {
        printf("Erreur d'initialisation des capteurs.\n");
        return -1;
    }

    // Initialisation BLE
    if (ble_init() != 0) {
        printf("Erreur d'initialisation BLE.\n");
        return -1;
    }

    while (1) {
        printf("\033[H\033[J"); // Rafraîchit la console
        printf("=== IKS01A3 DASHBOARD FULL ===\n\n");

        // Mise à jour des capteurs
        env_update(&env);
        motion_update(&imu);
        mag_update(&mag);

        // --- Affichage console ---
        printf("HTS221 : Temp: %.1f C | Hum: %.1f%%\n",
               sensor_value_to_double(&env.temp_hts),
               sensor_value_to_double(&env.humidity));
        printf("LPS22HH: Press: %.3f kPa | Temp: %.1f C\n\n",
               sensor_value_to_double(&env.pressure),
               sensor_value_to_double(&env.temp_lps));

        printf("LSM6DSO: Accel X: %.2f Y: %.2f Z: %.2f\n",
               sensor_value_to_double(&imu.accel[0]),
               sensor_value_to_double(&imu.accel[1]),
               sensor_value_to_double(&imu.accel[2]));

        printf("LIS2MDL: Magn  X: %.3f Y: %.3f Z: %.3f\n",
               sensor_value_to_double(&mag.magn[0]),
               sensor_value_to_double(&mag.magn[1]),
               sensor_value_to_double(&mag.magn[2]));

        // --- Préparation des données pour BLE (format CSV) ---
        char buffer[128];
        int len = snprintf(buffer, sizeof(buffer),
            "%.1f,%.1f,%.3f,%.1f,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f",
            sensor_value_to_double(&env.temp_hts),
            sensor_value_to_double(&env.humidity),
            sensor_value_to_double(&env.pressure),
            sensor_value_to_double(&env.temp_lps),
            sensor_value_to_double(&imu.accel[0]),
            sensor_value_to_double(&imu.accel[1]),
            sensor_value_to_double(&imu.accel[2]),
            sensor_value_to_double(&mag.magn[0]),
            sensor_value_to_double(&mag.magn[1]),
            sensor_value_to_double(&mag.magn[2]));

        // Envoi des données via BLE (notification)
        ble_notify_sensor_data((uint8_t*)buffer, len);

        k_sleep(K_MSEC(2000));
    }
    return 0;
}