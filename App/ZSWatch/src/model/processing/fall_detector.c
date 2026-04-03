#include "fall_detector.h"
#include <math.h>
#include <zephyr/kernel.h>

#define IMPACT_THRESHOLD 30.0   // m/s² (environ 3g)
#define POST_IMPACT_DURATION 2000 // ms
#define REST_THRESHOLD 1.2       // m/s²

void fall_detector_init(FallDetector *fd)
{
    fd->state = FALL_STATE_NORMAL;
    fd->impact_time = 0;
    fd->fall_detected = false;
}

bool fall_detector_update(FallDetector *fd, double ax, double ay, double az)
{
    double mag = sqrt(ax*ax + ay*ay + az*az);
    int64_t now = k_uptime_get();
    bool fall_now = false;

    switch (fd->state) {
        case FALL_STATE_NORMAL:
            if (mag > IMPACT_THRESHOLD) {
                fd->state = FALL_STATE_IMPACT;
                fd->impact_time = now;
            }
            break;

        case FALL_STATE_IMPACT:
            if (mag < REST_THRESHOLD) {
                fd->state = FALL_STATE_POST_IMPACT;
            } else if (now - fd->impact_time > 500) {
                fd->state = FALL_STATE_NORMAL;
            }
            break;

        case FALL_STATE_POST_IMPACT:
            if (mag < REST_THRESHOLD) {
                if (now - fd->impact_time > POST_IMPACT_DURATION) {
                    fd->fall_detected = true;
                    fall_now = true;
                    fd->state = FALL_STATE_NORMAL;
                }
            } else {
                fd->state = FALL_STATE_NORMAL;
            }
            break;
    }
    return fall_now;
}

bool fall_detector_get_and_clear(FallDetector *fd)
{
    bool ret = fd->fall_detected;
    fd->fall_detected = false;
    return ret;
}