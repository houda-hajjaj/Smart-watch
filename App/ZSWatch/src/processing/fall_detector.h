#ifndef FALL_DETECTOR_H
#define FALL_DETECTOR_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    FALL_STATE_NORMAL,
    FALL_STATE_IMPACT,
    FALL_STATE_POST_IMPACT
} fall_state_t;

typedef struct {
    fall_state_t state;
    int64_t impact_time;
    bool fall_detected;
} FallDetector;

void fall_detector_init(FallDetector *fd);
bool fall_detector_update(FallDetector *fd, double ax, double ay, double az);
bool fall_detector_get_and_clear(FallDetector *fd);

#endif