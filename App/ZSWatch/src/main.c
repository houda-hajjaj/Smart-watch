/**
 * @file main.c
 * @brief ZSWatch – point d'entrée principal (architecture multi-thread)
 *
 * Séquence d'initialisation :
 *   1. shared_data_init()         → met à zéro les structs partagées
 *   2. sensor_thread_init()       → initialise tous les capteurs HW
 *   3. processing_thread_init()   → initialise les modules de traitement
 *   4. ble_thread_init()          → initialise la pile BLE / GATT
 *   5. shared_data_timers_start() → démarre timer1 (100 ms) et timer0 (30 s)
 *   6. console_thread_start()          → lance le dashboard série (optionnel)
 *
 * Flux inter-threads :
 *
 *   timer1 (100ms ISR) ──► sem_sample_tick  ──► sensor_thread   (prio 5)
 *   sensor_thread      ──► sem_sensor_ready ──► processing_thread (prio 6)
 *   processing_thread  ──► sem_ble_notify   ──► ble_thread      (prio 7)
 *   timer0 (30s ISR)   ──► sem_sleep        ──► sleep_thread    (prio 3)
 *   k_sleep(500ms)     ──────────────────────── console_thread        (prio 8)
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "thread/shared_data.h"
#include "thread/sensor_thread.h"
#include "thread/processing_thread.h"
#include "thread/ble_thread.h"
#include "thread/console_thread.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    int err;

    /* 1 ── Initialiser les données partagées */
    shared_data_init();

    /* 2 ── Initialiser les capteurs physiques */
    err = sensor_thread_init();
    if (err) {
        LOG_ERR("sensor_thread_init echoue : %d", err);
        return err;
    }

    /* 3 ── Initialiser les modules de traitement */
    processing_thread_init();

    /* 4 ── Initialiser le service BLE (non bloquant si échec) */
    err = ble_thread_init();
    if (err) {
        LOG_WRN("ble_thread_init echoue : %d (BLE desactive)", err);
    }

    /* 5 ── Démarrer les timers → pipeline actif */
    shared_data_timers_start();

    /* 6 ── Lancer le dashboard série */
    console_thread_start();

    LOG_INF("ZSWatch demarre – pipeline multi-thread actif");

    return 0;
}