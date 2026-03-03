#ifndef WEATHER_H
#define WEATHER_H

typedef enum {
    WEATHER_UNKNOWN,
    WEATHER_STABLE,
    WEATHER_RISING,
    WEATHER_FALLING,
} weather_trend_t;

/**
 * @brief Initialise le module météo.
 */
void weather_init(void);

/**
 * @brief Met à jour la tendance à partir de la pression.
 * @param pressure_kpa Pression en kPa.
 * @return tendance.
 */
weather_trend_t weather_update(double pressure_kpa);

/**
 * @brief Retourne la tendance sous forme de chaîne.
 */
const char* weather_trend_to_str(weather_trend_t trend);

#endif /* WEATHER_H */