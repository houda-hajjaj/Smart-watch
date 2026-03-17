/**
 * @file console_thread.c
 * @brief Thread d'affichage série – dashboard terminal via printk().
 *
 * Corrections v2 :
 *   - printf() → printk() : fonctionne sans CONFIG_STDOUT_CONSOLE
 *   - Suppression du clear VT100 (\033[H\033[J) : incompatible avec
 *     le backend RTT/UART de Zephyr et pollue le log
 *   - Séparateur visuel entre chaque refresh pour faciliter la lecture
 *
 * Priorité 8 (plus basse de l'application) : s'exécute uniquement
 * quand tous les autres threads sont bloqués sur leurs sémaphores.
 */

#include "console_thread.h"
#include "shared_data.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "processing/compass.h"
#include "processing/activity.h"
#include "processing/weather.h"
#include "ble/ble_service.h"

LOG_MODULE_REGISTER(console_thread, LOG_LEVEL_INF);

/* ── Paramètres ─────────────────────────────────────────────────── */
#define CONSOLE_STACK_SIZE   1024
#define CONSOLE_PRIORITY     8
#define CONSOLE_REFRESH_MS   500

/* ════════════════════════════════════════════════════════════════
 *  Thread function
 * ════════════════════════════════════════════════════════════════ */
static void console_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    struct watch_raw_data       raw;
    struct watch_processed_data proc;

    while (1) {
        k_sleep(K_MSEC(CONSOLE_REFRESH_MS));

        k_mutex_lock(&mutex_data, K_FOREVER);
        raw  = g_raw;
        proc = g_processed;
        k_mutex_unlock(&mutex_data);

        /* Séparateur lisible entre chaque snapshot */
        printk("\n--- ZSWatch [%02d:%02d:%02d] ---\n",
               raw.time.tm_hour,
               raw.time.tm_min,
               raw.time.tm_sec);

        /* Environnement */
        printk("Temp: %.1f C  Hum: %.1f%%  Press: %.3f kPa\n",
               raw.temperature_c,
               raw.humidity_pct,
               raw.pressure_kpa);
        printk("Altitude: %.1f m  Etages: %d\n",
               proc.altitude_m,
               proc.floors);

        /* IMU */
        printk("Accel: X=%.2f Y=%.2f Z=%.2f m/s2\n",
               sensor_value_to_double(&raw.accel[0]),
               sensor_value_to_double(&raw.accel[1]),
               sensor_value_to_double(&raw.accel[2]));
        printk("Magn : X=%.3f Y=%.3f Z=%.3f uT\n",
               sensor_value_to_double(&raw.magn[0]),
               sensor_value_to_double(&raw.magn[1]),
               sensor_value_to_double(&raw.magn[2]));

        /* Orientation */
        printk("Cap: %.1f deg (%s)  Roll=%.1f Pitch=%.1f Yaw=%.1f\n",
               proc.heading,
               compass_direction_to_str(proc.heading),
               proc.orientation.roll,
               proc.orientation.pitch,
               proc.orientation.yaw);

        /* Activité */
        printk("Activite: %s%s\n",
               activity_to_str(proc.activity),
               proc.fall_detected ? "  !! CHUTE !!" : "");

        /* Météo + fitness */
        printk("Pression: %s\n", weather_trend_to_str(proc.weather));
        printk("Pas: %u  Dist: %.2f km  Cal: %.1f kcal\n",
               proc.steps,
               proc.distance_m / 1000.0,
               proc.calories);

        /* BLE */
        printk("BLE: %s\n",
               ble_service_is_connected() ? "Connecte" : "En attente");
    }
}

K_THREAD_DEFINE(console_tid, CONSOLE_STACK_SIZE,
                console_thread_fn, NULL, NULL, NULL,
                CONSOLE_PRIORITY, 0, K_TICKS_FOREVER);

/* ════════════════════════════════════════════════════════════════
 *  API publique
 * ════════════════════════════════════════════════════════════════ */
void console_thread_start(void)
{
    k_thread_resume(console_tid);
    LOG_INF("Dashboard serie actif (refresh=%d ms)", CONSOLE_REFRESH_MS);
}