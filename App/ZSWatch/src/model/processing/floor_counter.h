#ifndef FLOOR_COUNTER_H
#define FLOOR_COUNTER_H

#include <stdbool.h>

typedef struct {
    int floor_count;          // nombre d'étages montés (positif) ou descentes (négatif)
    double last_altitude;     // dernière altitude connue
    bool initialized;
} FloorCounter;

void floor_counter_init(FloorCounter *fc);
void floor_counter_update(FloorCounter *fc, double altitude_m);
int floor_counter_get(const FloorCounter *fc);

#endif