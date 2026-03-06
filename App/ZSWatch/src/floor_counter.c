#include "floor_counter.h"
#include <math.h>

#define FLOOR_HEIGHT 3.0   // hauteur moyenne d'un étage en mètres

void floor_counter_init(FloorCounter *fc)
{
    fc->floor_count = 0;
    fc->last_altitude = 0.0;
    fc->initialized = false;
}

void floor_counter_update(FloorCounter *fc, double altitude_m)
{
    if (!fc->initialized) {
        fc->last_altitude = altitude_m;
        fc->initialized = true;
        return;
    }

    double diff = altitude_m - fc->last_altitude;
    if (fabs(diff) >= FLOOR_HEIGHT / 2.0) {
        int floors = (int)(diff / FLOOR_HEIGHT);
        fc->floor_count += floors;
        fc->last_altitude = altitude_m;
    }
}

int floor_counter_get(const FloorCounter *fc)
{
    return fc->floor_count;
}