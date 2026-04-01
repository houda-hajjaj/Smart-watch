#include "calories.h"

#define CAL_WALKING  0.5   // kcal/kg/km
#define CAL_RUNNING  1.0   // kcal/kg/km

void calories_init(CaloriesCounter *cc, double weight_kg)
{
    cc->weight_kg = weight_kg;
    cc->total_calories = 0.0;
}

void calories_add_distance(CaloriesCounter *cc, double distance_m, activity_t activity)
{
    double km = distance_m / 1000.0;
    double coeff = 0.0;
    switch (activity) {
        case ACTIVITY_WALKING:
            coeff = CAL_WALKING;
            break;
        case ACTIVITY_RUNNING:
            coeff = CAL_RUNNING;
            break;
        default:
            coeff = 0.0;
            break;
    }
    cc->total_calories += km * cc->weight_kg * coeff;
}

double calories_get(const CaloriesCounter *cc)
{
    return cc->total_calories;
}

void calories_reset(CaloriesCounter *cc)
{
    cc->total_calories = 0.0;
}