#include "weather.h"
#include <math.h>
#include <zephyr/kernel.h>

#define HISTORY_SIZE 10
static float pressure_history[HISTORY_SIZE];
static int hist_index = 0;
static int hist_count = 0;

void weather_init(void)
{
    for (int i = 0; i < HISTORY_SIZE; i++) {
        pressure_history[i] = 0.0f;
    }
    hist_index = 0;
    hist_count = 0;
}

weather_trend_t weather_update(double pressure_kpa)
{
    pressure_history[hist_index] = (float)pressure_kpa;
    hist_index = (hist_index + 1) % HISTORY_SIZE;
    if (hist_count < HISTORY_SIZE) hist_count++;

    if (hist_count < 2) {
        return WEATHER_UNKNOWN;
    }

    // Calculer la pente (régression linéaire simple)
    float sum_x = 0.0f, sum_y = 0.0f, sum_xy = 0.0f, sum_x2 = 0.0f;
    int n = hist_count;
    for (int i = 0; i < n; i++) {
        float x = (float)i;
        float y = pressure_history[i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }
    float slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);

    // Seuils en kPa par échantillon (à ajuster)
    if (fabs(slope) < 0.001f) {
        return WEATHER_STABLE;
    } else if (slope > 0.001f) {
        return WEATHER_RISING;
    } else {
        return WEATHER_FALLING;
    }
}

const char* weather_trend_to_str(weather_trend_t trend)
{
    switch (trend) {
        case WEATHER_UNKNOWN: return "Inconnue";
        case WEATHER_STABLE:  return "Stable";
        case WEATHER_RISING:  return "Amélioration (hausse)";
        case WEATHER_FALLING: return "Détérioration (baisse)";
        default: return "?";
    }
}