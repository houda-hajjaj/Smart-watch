#ifndef RTC_H
#define RTC_H

#include <zephyr/drivers/rtc.h>
#include <stdbool.h>

typedef struct {
    const struct device *dev;
} WatchRTC;

/**
 * @brief Initialise le module RTC (RV-8263-C8).
 * @return 0 si succès, négatif sinon.
 */
int watch_rtc_init(WatchRTC *r);

/**
 * @brief Règle la date et l'heure.
 * @param year  Année complète 
 * @param month Mois 1-12
 * @param day   Jour 1-31
 * @param hour  Heure 0-23
 * @param min   Minute 0-59
 * @param sec   Seconde 0-59
 * @return 0 si succès, négatif sinon.
 */
int watch_rtc_set(WatchRTC *r, int year, int month, int day,
                  int hour, int min, int sec);

/**
 * @brief Lit la date et l'heure courante.
 * @param t Pointeur vers struct rtc_time à remplir.
 * @return 0 si succès, négatif sinon.
 */
int watch_rtc_get(WatchRTC *r, struct rtc_time *t);

/**
 * @brief Affiche la date/heure sur la console.
 */
void watch_rtc_print(const struct rtc_time *t);

#endif /* RTC_H */
