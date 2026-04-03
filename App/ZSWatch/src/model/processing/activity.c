#include "activity.h"
#include <math.h>

// Seuils empiriques (à ajuster)
#define WALKING_THRESHOLD 1.5  // g
#define RUNNING_THRESHOLD 3.0  // g

activity_t activity_update(const struct sensor_value accel[3])
{
    double ax = sensor_value_to_double(&accel[0]);
    double ay = sensor_value_to_double(&accel[1]);
    double az = sensor_value_to_double(&accel[2]);

    // Magnitude en g (1g = 9.81 m/s²)
    double mag = sqrt(ax*ax + ay*ay + az*az) / 9.81;

    if (mag < 1.1) {
        return ACTIVITY_REST;
    } else if (mag < RUNNING_THRESHOLD) {
        return ACTIVITY_WALKING;
    } else {
        return ACTIVITY_RUNNING;
    }
}

const char* activity_to_str(activity_t act)
{
    switch (act) {
        case ACTIVITY_REST:    return "Repos";
        case ACTIVITY_WALKING: return "Marche";
        case ACTIVITY_RUNNING: return "Course";
        default:               return "Inconnu";
    }
}