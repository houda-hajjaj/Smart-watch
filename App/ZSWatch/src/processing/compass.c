#include "compass.h"
#include <math.h>

#define PI 3.14159265358979323846

void compass_init(Compass *comp)
{
    comp->heading = 0.0;
    comp->x = comp->y = comp->z = 0.0;
    // Si calibration, mettre les offsets à zéro
    comp->offset_x = comp->offset_y = comp->offset_z = 0.0;
}

void compass_update(Compass *comp, const struct sensor_value magn[3])
{
    // Convertir les valeurs en double
    double mx = sensor_value_to_double(&magn[0]);
    double my = sensor_value_to_double(&magn[1]);
    double mz = sensor_value_to_double(&magn[2]);

    // Appliquer les offsets de calibration si nécessaire
    mx -= comp->offset_x;
    my -= comp->offset_y;
    mz -= comp->offset_z;

    comp->x = mx;
    comp->y = my;
    comp->z = mz;

    // Calcul de l'angle (en radians) en utilisant X et Y
    double angle_rad = atan2(my, mx);
    // Conversion en degrés et mise dans l'intervalle [0, 360)
    double angle_deg = angle_rad * 180.0 / PI;
    if (angle_deg < 0) {
        angle_deg += 360.0;
    }
    comp->heading = angle_deg;
}

double compass_get_heading(const Compass *comp)
{
    return comp->heading;
}
const char* compass_direction_to_str(double heading)
{
    const char* directions[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
                                "S", "SSO", "SO", "OSO", "O", "ONO", "NO", "NNO"};
    int index = (int)((heading + 11.25) / 22.5) % 16;
    return directions[index];
}