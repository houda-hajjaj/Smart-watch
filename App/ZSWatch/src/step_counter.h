#ifndef STEP_COUNTER_H
#define STEP_COUNTER_H

#include <zephyr/drivers/sensor.h>
#include <stdbool.h>

/* Paramètres ajustables */
#define STEP_MIN_INTERVAL_MS    250   /* Intervalle minimum entre 2 pas (ms) */

typedef enum {
    STEP_STATE_IDLE,
    STEP_STATE_PEAK,
} step_state_t;

typedef struct {
    uint32_t step_count;
    step_state_t state;
    int64_t last_step_time_ms;
    double last_magnitude;        // dernière magnitude filtrée (pour affichage)
    double filtered_magnitude;    // magnitude après filtre passe-bas
    double mean_magnitude;        // moyenne mobile
    double buffer[10];            // buffer circulaire pour la moyenne
    int buffer_index;
    double peak_threshold;        // seuil haut adaptatif
    double valley_threshold;       // seuil bas adaptatif
} StepCounter;

void step_counter_init(StepCounter *sc);
void step_counter_reset(StepCounter *sc);
void step_counter_update(StepCounter *sc, const struct sensor_value accel[3]);
uint32_t step_counter_get_steps(const StepCounter *sc);

#endif /* STEP_COUNTER_H */