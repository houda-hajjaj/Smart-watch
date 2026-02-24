#include "step_counter.h"
#include <zephyr/kernel.h>
#include <math.h>
#include <stdio.h>

#define GRAVITY_MS2  9.81
#define ALPHA         0.2   // Coefficient du filtre passe-bas (0 < alpha < 1)
#define WINDOW_SIZE   10    // Nombre d'échantillons pour la moyenne mobile

void step_counter_init(StepCounter *sc)
{
    sc->step_count = 0;
    sc->state = STEP_STATE_IDLE;
    sc->last_step_time_ms = 0;
    sc->last_magnitude = 0.0;
    sc->filtered_magnitude = 1.0; // valeur typique au repos
    sc->mean_magnitude = 1.0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sc->buffer[i] = 1.0;
    }
    sc->buffer_index = 0;
    sc->peak_threshold = 1.2;
    sc->valley_threshold = 0.8;
}

void step_counter_reset(StepCounter *sc)
{
    step_counter_init(sc);
}

void step_counter_update(StepCounter *sc, const struct sensor_value accel[3])
{
    double ax = sensor_value_to_double(&accel[0]);
    double ay = sensor_value_to_double(&accel[1]);
    double az = sensor_value_to_double(&accel[2]);

    // Magnitude brute en g
    double raw_mag = sqrt(ax*ax + ay*ay + az*az) / GRAVITY_MS2;

    // Filtre passe-bas exponentiel
    sc->filtered_magnitude = ALPHA * raw_mag + (1 - ALPHA) * sc->filtered_magnitude;

    // Mise à jour de la moyenne mobile
    sc->buffer[sc->buffer_index] = sc->filtered_magnitude;
    sc->buffer_index = (sc->buffer_index + 1) % WINDOW_SIZE;
    double sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sum += sc->buffer[i];
    }
    sc->mean_magnitude = sum / WINDOW_SIZE;

    // Ajustement dynamique des seuils : par exemple, 1.2 * moyenne et 0.8 * moyenne
    sc->peak_threshold = sc->mean_magnitude * 1.2;
    sc->valley_threshold = sc->mean_magnitude * 0.8;

    sc->last_magnitude = sc->filtered_magnitude; // pour affichage

    int64_t now = k_uptime_get();

    switch (sc->state) {
    case STEP_STATE_IDLE:
        if (sc->filtered_magnitude > sc->peak_threshold) {
            sc->state = STEP_STATE_PEAK;
        }
        break;

    case STEP_STATE_PEAK:
        if (sc->filtered_magnitude < sc->valley_threshold) {
            if ((now - sc->last_step_time_ms) >= STEP_MIN_INTERVAL_MS) {
                sc->step_count++;
                sc->last_step_time_ms = now;
            }
            sc->state = STEP_STATE_IDLE;
        }
        break;
    }
}

uint32_t step_counter_get_steps(const StepCounter *sc)
{
    return sc->step_count;
}