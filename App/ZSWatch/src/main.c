#include <zephyr/kernel.h>
#include <stdio.h>
#include "motion_sensor.h"
#include "mag_sensor.h"
#include "env_sensor.h"
#include "step_counter.h"
#include "ble_service.h"

int main(void) {
    MotionSensor imu;
    MagSensor mag;
    EnvSensor env;
    StepCounter steps;

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

        // Mise à jour du compteur de pas
        step_counter_update(&steps, imu.accel);
        printf("\nPedometre: %u pas (mag: %.2f g)\n",
               step_counter_get_steps(&steps), steps.last_magnitude);

        // ── Envoi BLE ──
        // Formats BLE SIG : temp=0.01°C, hum=0.01%, press=0.1Pa
        struct ble_sensor_data ble_data = {
            .temperature = (int16_t)(sensor_value_to_double(&env.temp_hts) * 100),
            .humidity    = (uint16_t)(sensor_value_to_double(&env.humidity) * 100),
            .pressure    = (uint32_t)(sensor_value_to_double(&env.pressure) * 10000), /* kPa → 0.1 Pa */
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