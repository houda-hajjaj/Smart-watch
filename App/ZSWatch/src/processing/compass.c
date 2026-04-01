#include "compass.h"

#include <math.h>

#define PI 3.14159265358979323846
#define COMPASS_ACCEL_ALPHA 0.20
#define COMPASS_MAG_ALPHA 0.15
#define COMPASS_HEADING_ALPHA 0.18
#define COMPASS_CAL_MIN_SPAN_UT 8.0
#define COMPASS_MIN_HORIZONTAL_FIELD 0.05

static double low_pass(double prev, double sample, double alpha)
{
    return prev + alpha * (sample - prev);
}

static double wrap_degrees(double degrees)
{
    while (degrees < 0.0) {
        degrees += 360.0;
    }
    while (degrees >= 360.0) {
        degrees -= 360.0;
    }
    return degrees;
}

static double shortest_angle_delta(double target, double current)
{
    double delta = target - current;

    while (delta > 180.0) {
        delta -= 360.0;
    }
    while (delta < -180.0) {
        delta += 360.0;
    }

    return delta;
}

void compass_init(Compass *comp)
{
    comp->heading = 0.0;
    comp->x = comp->y = comp->z = 0.0;
    comp->offset_x = comp->offset_y = comp->offset_z = 0.0;
    comp->scale_x = comp->scale_y = comp->scale_z = 1.0;
    comp->min_x = comp->min_y = comp->min_z = 0.0;
    comp->max_x = comp->max_y = comp->max_z = 0.0;
    comp->accel_x = comp->accel_y = comp->accel_z = 0.0;
    comp->mag_x = comp->mag_y = comp->mag_z = 0.0;
    comp->initialized = false;
    comp->heading_valid = false;
}

void compass_update(Compass *comp,
                    const struct sensor_value accel[3],
                    const struct sensor_value magn[3])
{
    double ax = sensor_value_to_double(&accel[0]);
    double ay = sensor_value_to_double(&accel[1]);
    double az = sensor_value_to_double(&accel[2]);
    double mx = sensor_value_to_double(&magn[0]);
    double my = sensor_value_to_double(&magn[1]);
    double mz = sensor_value_to_double(&magn[2]);
    double accel_norm;
    double roll;
    double pitch;
    double xh;
    double yh;
    double heading_deg;
    double span_x;
    double span_y;
    double span_z;
    double avg_span;
    double horizontal_field;

    if (!comp->initialized) {
        comp->accel_x = ax;
        comp->accel_y = ay;
        comp->accel_z = az;
        comp->mag_x = mx;
        comp->mag_y = my;
        comp->mag_z = mz;
        comp->min_x = comp->max_x = mx;
        comp->min_y = comp->max_y = my;
        comp->min_z = comp->max_z = mz;
        comp->initialized = true;
    } else {
        comp->accel_x = low_pass(comp->accel_x, ax, COMPASS_ACCEL_ALPHA);
        comp->accel_y = low_pass(comp->accel_y, ay, COMPASS_ACCEL_ALPHA);
        comp->accel_z = low_pass(comp->accel_z, az, COMPASS_ACCEL_ALPHA);
        comp->mag_x = low_pass(comp->mag_x, mx, COMPASS_MAG_ALPHA);
        comp->mag_y = low_pass(comp->mag_y, my, COMPASS_MAG_ALPHA);
        comp->mag_z = low_pass(comp->mag_z, mz, COMPASS_MAG_ALPHA);
    }

    if (comp->mag_x < comp->min_x) comp->min_x = comp->mag_x;
    if (comp->mag_x > comp->max_x) comp->max_x = comp->mag_x;
    if (comp->mag_y < comp->min_y) comp->min_y = comp->mag_y;
    if (comp->mag_y > comp->max_y) comp->max_y = comp->mag_y;
    if (comp->mag_z < comp->min_z) comp->min_z = comp->mag_z;
    if (comp->mag_z > comp->max_z) comp->max_z = comp->mag_z;

    span_x = comp->max_x - comp->min_x;
    span_y = comp->max_y - comp->min_y;
    span_z = comp->max_z - comp->min_z;

    if ((span_x > COMPASS_CAL_MIN_SPAN_UT) &&
        (span_y > COMPASS_CAL_MIN_SPAN_UT) &&
        (span_z > COMPASS_CAL_MIN_SPAN_UT)) {
        avg_span = (span_x + span_y + span_z) / 3.0;

        comp->offset_x = (comp->max_x + comp->min_x) * 0.5;
        comp->offset_y = (comp->max_y + comp->min_y) * 0.5;
        comp->offset_z = (comp->max_z + comp->min_z) * 0.5;
        comp->scale_x = avg_span / span_x;
        comp->scale_y = avg_span / span_y;
        comp->scale_z = avg_span / span_z;
    }

    mx = (comp->mag_x - comp->offset_x) * comp->scale_x;
    my = (comp->mag_y - comp->offset_y) * comp->scale_y;
    mz = (comp->mag_z - comp->offset_z) * comp->scale_z;

    accel_norm = sqrt(comp->accel_x * comp->accel_x +
                      comp->accel_y * comp->accel_y +
                      comp->accel_z * comp->accel_z);
    if (accel_norm < 0.001) {
        return;
    }

    ax = comp->accel_x / accel_norm;
    ay = comp->accel_y / accel_norm;
    az = comp->accel_z / accel_norm;

    roll = atan2(ay, az);
    pitch = atan2(-ax, sqrt(ay * ay + az * az));

    xh = mx * cos(pitch) + mz * sin(pitch);
    yh = mx * sin(roll) * sin(pitch) +
         my * cos(roll) -
         mz * sin(roll) * cos(pitch);

    horizontal_field = sqrt(xh * xh + yh * yh);
    if (horizontal_field < COMPASS_MIN_HORIZONTAL_FIELD) {
        xh = mx;
        yh = my;
    }

    heading_deg = wrap_degrees(atan2(yh, xh) * 180.0 / PI);

    comp->x = xh;
    comp->y = yh;
    comp->z = mz;

    if (!comp->heading_valid) {
        comp->heading = heading_deg;
        comp->heading_valid = true;
    } else {
        comp->heading = wrap_degrees(comp->heading +
            COMPASS_HEADING_ALPHA * shortest_angle_delta(heading_deg, comp->heading));
    }
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
