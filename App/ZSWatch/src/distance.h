#ifndef DISTANCE_H
#define DISTANCE_H

#include <stdint.h>

typedef struct {
    double stride_length;   // longueur de foulée en mètres
    double total_distance;  // distance totale en mètres
} DistanceCounter;

void distance_init(DistanceCounter *dc, double stride_length);
void distance_add_steps(DistanceCounter *dc, uint32_t steps_added);
double distance_get(const DistanceCounter *dc);
void distance_reset(DistanceCounter *dc);

#endif