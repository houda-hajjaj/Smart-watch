#include "step_counter.h"
#include <zephyr/kernel.h>
#include <math.h>
#include <stdio.h>

/**
 * Algorithme de détection de pas :
 *
 * 1. Calculer la magnitude du vecteur accélération :
 *    mag = sqrt(ax² + ay² + az²)
 *    (convertie en g, 1g ≈ 9.81 m/s²)
 *
 * 2. Machine à états à 2 niveaux :
 *    - IDLE  : on attend que mag > THRESHOLD_HIGH → passage à PEAK
 *    - PEAK  : on attend que mag < THRESHOLD_LOW  → un pas est validé,
 *              retour à IDLE
 *
 * 3. Filtrage temporel : un pas n'est compté que si au moins
 *    STEP_MIN_INTERVAL_MS ms se sont écoulées depuis le dernier pas.
 */

#define GRAVITY_MS2  9.81

void step_counter_init(StepCounter *sc)
{
    sc->step_count = 0;
    sc->state = STEP_STATE_IDLE;
    sc->last_step_time_ms = 0;
    sc->last_magnitude = 0.0;
}

void step_counter_reset(StepCounter *sc)
{
    step_counter_init(sc);
}

void step_counter_update(StepCounter *sc, const struct sensor_value accel[3])
{
    /* Convertir sensor_value → double (m/s²) puis en g */
    double ax = sensor_value_to_double(&accel[0]);
    double ay = sensor_value_to_double(&accel[1]);
    double az = sensor_value_to_double(&accel[2]);

    double magnitude = sqrt(ax * ax + ay * ay + az * az) / GRAVITY_MS2;
    sc->last_magnitude = magnitude;

    int64_t now = k_uptime_get();

    switch (sc->state) {
    case STEP_STATE_IDLE:
        /* Attendre que l'accélération dépasse le seuil haut */
        if (magnitude > STEP_THRESHOLD_HIGH) {
            sc->state = STEP_STATE_PEAK;
        }
        break;

    case STEP_STATE_PEAK:
        /* Attendre que l'accélération redescende sous le seuil bas */
        if (magnitude < STEP_THRESHOLD_LOW) {
            /* Vérifier l'intervalle minimum entre deux pas */
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