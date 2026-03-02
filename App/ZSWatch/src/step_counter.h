#ifndef STEP_COUNTER_H
#define STEP_COUNTER_H

#include <zephyr/drivers/sensor.h>
#include <stdbool.h>

/**
 * Compteur de pas (pédomètre) basé sur la détection de pics
 * d'accélération du capteur LSM6DSO.
 *
 * Algorithme : seuil dynamique sur la magnitude de l'accélération
 * avec filtrage temporel pour éviter les doubles comptages.
 */

/* Paramètres ajustables */
#define STEP_THRESHOLD_HIGH     1.2   /* Seuil haut (g) pour détecter un pic */
#define STEP_THRESHOLD_LOW      0.8   /* Seuil bas (g) pour valider la descente */
#define STEP_MIN_INTERVAL_MS    250   /* Intervalle minimum entre 2 pas (ms) */

typedef enum {
    STEP_STATE_IDLE,      /* En attente d'un pic */
    STEP_STATE_PEAK,      /* Pic détecté, attente de descente */
} step_state_t;

typedef struct {
    uint32_t step_count;          /* Nombre total de pas */
    step_state_t state;           /* État de la machine à états */
    int64_t last_step_time_ms;    /* Timestamp du dernier pas validé */
    double last_magnitude;        /* Dernière magnitude calculée */
} StepCounter;

/**
 * @brief Initialise le compteur de pas.
 * @param sc Pointeur vers la structure StepCounter.
 */
void step_counter_init(StepCounter *sc);

/**
 * @brief Réinitialise le compteur de pas à zéro.
 * @param sc Pointeur vers la structure StepCounter.
 */
void step_counter_reset(StepCounter *sc);

/**
 * @brief Met à jour le compteur de pas avec les nouvelles valeurs
 *        d'accélération. Doit être appelé à chaque cycle de lecture
 *        du capteur.
 * @param sc   Pointeur vers la structure StepCounter.
 * @param accel Tableau de 3 sensor_value (X, Y, Z) en m/s².
 */
void step_counter_update(StepCounter *sc, const struct sensor_value accel[3]);

/**
 * @brief Retourne le nombre total de pas comptés.
 * @param sc Pointeur vers la structure StepCounter.
 * @return Nombre de pas.
 */
uint32_t step_counter_get_steps(const StepCounter *sc);

#endif /* STEP_COUNTER_H */