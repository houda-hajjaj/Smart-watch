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
    ble_init(); // Ne retourne pas de code d'erreur (log interne)

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

        // --- Envoi des données via BLE (caractéristiques standard) ---

        // Température HTS221 (°C * 100)
        ble_update_temperature((int16_t)(sensor_value_to_double(&env.temp_hts) * 100));

        // Humidité (% * 100)
        ble_update_humidity((uint16_t)(sensor_value_to_double(&env.humidity) * 100));

        // Pression : conversion kPa -> Pa (x1000)
        ble_update_pressure((uint32_t)(sensor_value_to_double(&env.pressure) * 1000));

        // Accélération (G * 100, à ajuster selon l'unité souhaitée)
        ble_update_acceleration(
            (int16_t)(sensor_value_to_double(&imu.accel[0]) * 100),
            (int16_t)(sensor_value_to_double(&imu.accel[1]) * 100),
            (int16_t)(sensor_value_to_double(&imu.accel[2]) * 100)
        );

        // Magnétomètre (µT * 100, à ajuster)
        ble_update_magnetometer(
            (int16_t)(sensor_value_to_double(&mag.magn[0]) * 100),
            (int16_t)(sensor_value_to_double(&mag.magn[1]) * 100),
            (int16_t)(sensor_value_to_double(&mag.magn[2]) * 100)
        );

        k_sleep(K_MSEC(2000));
    }
    return 0;
}