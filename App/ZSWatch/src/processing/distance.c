#include "distance.h"

void distance_init(DistanceCounter *dc, double stride_length)
{
    dc->stride_length = stride_length;
    dc->total_distance = 0.0;
}

void distance_add_steps(DistanceCounter *dc, uint32_t steps_added)
{
    dc->total_distance += steps_added * dc->stride_length;
}

double distance_get(const DistanceCounter *dc)
{
    return dc->total_distance;
}

void distance_reset(DistanceCounter *dc)
{
    dc->total_distance = 0.0;
}