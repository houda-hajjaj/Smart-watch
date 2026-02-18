#include <zephyr/kernel.h>
#include <stdio.h>
#include "motion_sensor.h"
#include "mag_sensor.h"
#include "env_sensor.h"

int main(void) {
    MotionSensor imu;
    MagSensor mag;
    EnvSensor env;

    // Initialize sensors
    if (motion_init(&imu) != 0 || mag_init(&mag) != 0 || env_init(&env) != 0) {
        printf("Erreur d'initialisation des capteurs.\n");
        return -1;
    }

    while (1) {
        printf("\033[H\033[J"); // Pour refresher la console à chaque itération
        printf("=== IKS01A3 DASHBOARD FULL ===\n\n");

        // --- Données Environnement ---
        if (env_update(&env) == 0) {
            printf("HTS221 : Temp: %.1f C | Hum: %.1f%%\n",
                   sensor_value_to_double(&env.temp_hts),
                   sensor_value_to_double(&env.humidity));
            printf("LPS22HH: Press: %.3f kPa | Temp: %.1f C\n\n",
                   sensor_value_to_double(&env.pressure),
                   sensor_value_to_double(&env.temp_lps));
        }

        // --- Données Mouvement (LSM6DSO) ---
        if (motion_update(&imu) == 0) {
            printf("LSM6DSO: Accel X: %.2f Y: %.2f Z: %.2f\n",
                   sensor_value_to_double(&imu.accel[0]),
                   sensor_value_to_double(&imu.accel[1]),
                   sensor_value_to_double(&imu.accel[2]));
        }

        // --- Données Magnétiques (LIS2MDL) ---
        if (mag_update(&mag) == 0) {
            printf("LIS2MDL: Magn  X: %.3f Y: %.3f Z: %.3f\n",
                   sensor_value_to_double(&mag.magn[0]),
                   sensor_value_to_double(&mag.magn[1]),
                   sensor_value_to_double(&mag.magn[2]));
        }

        k_sleep(K_MSEC(2000));
    }
    return 0;
}