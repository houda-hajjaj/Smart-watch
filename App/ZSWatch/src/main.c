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
#include "floor_counter.h"
#include "distance.h"
#include "calories.h"
#include "fall_detector.h"

int main(void) {
    MotionSensor imu;
    MagSensor mag;
    EnvSensor env;
    StepCounter steps;
    Compass comp;
    Altimeter alt;
    FloorCounter floor_counter;
    DistanceCounter distance;
    CaloriesCounter calories;
    FallDetector fall_detector;

    compass_init(&comp);
    altimeter_init(&alt);
    fusion_init();
    weather_init();
    floor_counter_init(&floor_counter);
    distance_init(&distance, 0.75);      // foulée par défaut 75 cm
    calories_init(&calories, 70.0);      // poids par défaut 70 kg
    fall_detector_init(&fall_detector);

    if (motion_init(&imu) != 0 || mag_init(&mag) != 0 || env_init(&env) != 0) {
        printf("Erreur d'initialisation des capteurs.\n");
        return -1;
    }

    step_counter_init(&steps);

    if (ble_service_init() != 0) {
        printf("Erreur d'initialisation BLE.\n");
    }

    uint32_t last_steps = 0;

    while (1) {
        printf("\033[H\033[J");
        printf("=== IKS01A3 DASHBOARD FULL ===\n\n");

        env_update(&env);
        motion_update(&imu);
        mag_update(&mag);

        altimeter_update(&alt, sensor_value_to_double(&env.pressure));
        floor_counter_update(&floor_counter, altimeter_get_altitude(&alt));

        double ax = sensor_value_to_double(&imu.accel[0]);
        double ay = sensor_value_to_double(&imu.accel[1]);
        double az = sensor_value_to_double(&imu.accel[2]);

        if (fall_detector_update(&fall_detector, ax, ay, az)) {
            printf("⚠️ Chute détectée !\n");
        }

        printf("HTS221 : Temp: %.1f C | Hum: %.1f%%\n",
               sensor_value_to_double(&env.temp_hts),
               sensor_value_to_double(&env.humidity));
        printf("LPS22HH: Press: %.3f kPa | Temp: %.1f C\n",
               sensor_value_to_double(&env.pressure),
               sensor_value_to_double(&env.temp_lps));
        printf("Altitude: %.1f m\n", altimeter_get_altitude(&alt));
        printf("Étages: %d\n\n", floor_counter_get(&floor_counter));

        printf("LSM6DSO: Accel X: %.2f Y: %.2f Z: %.2f\n", ax, ay, az);
        printf("LIS2MDL: Magn  X: %.3f Y: %.3f Z: %.3f\n",
               sensor_value_to_double(&mag.magn[0]),
               sensor_value_to_double(&mag.magn[1]),
               sensor_value_to_double(&mag.magn[2]));

        compass_update(&comp, mag.magn);
        printf("Boussole: %.1f° (%s)\n",
               compass_get_heading(&comp),
               compass_direction_to_str(compass_get_heading(&comp)));

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

        step_counter_update(&steps, imu.accel);
        uint32_t current_steps = step_counter_get_steps(&steps);
        uint32_t steps_diff = current_steps - last_steps;
        if (steps_diff > 0) {
            distance_add_steps(&distance, steps_diff);
            calories_add_distance(&calories, steps_diff * distance.stride_length, act);
            last_steps = current_steps;
        }
        printf("Pas: %u\n", current_steps);
        printf("Distance: %.2f km\n", distance_get(&distance) / 1000.0);
        printf("Calories: %.1f kcal\n", calories_get(&calories));

        struct ble_sensor_data ble_data = {
            .temperature = (int16_t)(sensor_value_to_double(&env.temp_hts) * 100),
            .humidity    = (uint16_t)(sensor_value_to_double(&env.humidity) * 100),
            .pressure    = (uint32_t)(sensor_value_to_double(&env.pressure) * 10000),
            .accel = {
                (int16_t)(ax * 1000),
                (int16_t)(ay * 1000),
                (int16_t)(az * 1000),
            },
            .magn = {
                (int16_t)(sensor_value_to_double(&mag.magn[0]) * 1000),
                (int16_t)(sensor_value_to_double(&mag.magn[1]) * 1000),
                (int16_t)(sensor_value_to_double(&mag.magn[2]) * 1000),
            },
            .steps = current_steps,
        };
        ble_service_update(&ble_data);

        printf("BLE: %s\n", ble_service_is_connected() ? "Connecte" : "En attente...");

        k_sleep(K_MSEC(200));
    }
    return 0;
}