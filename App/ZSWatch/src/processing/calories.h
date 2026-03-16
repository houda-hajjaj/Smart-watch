#ifndef CALORIES_H
#define CALORIES_H

#include <stdint.h>
#include "activity.h"   // pour activity_t

typedef struct {
    double weight_kg;        // poids de l'utilisateur en kg
    double total_calories;   // calories totales brûlées
} CaloriesCounter;

void calories_init(CaloriesCounter *cc, double weight_kg);
void calories_add_distance(CaloriesCounter *cc, double distance_m, activity_t activity);
double calories_get(const CaloriesCounter *cc);
void calories_reset(CaloriesCounter *cc);

#endif