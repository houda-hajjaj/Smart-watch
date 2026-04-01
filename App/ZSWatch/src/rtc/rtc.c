#include "rtc.h"
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <stdio.h>
#include <errno.h>

int watch_rtc_init(WatchRTC *r)
{
    r->dev = DEVICE_DT_GET(DT_NODELABEL(rv8263));
    if (!device_is_ready(r->dev)) {
        return -ENODEV;
    }
    return 0;
}

int watch_rtc_set(WatchRTC *r, int year, int month, int day,
                  int hour, int min, int sec)
{
    struct rtc_time t = {
        .tm_year = year - 1900,
        .tm_mon  = month - 1,
        .tm_mday = day,
        .tm_hour = hour,
        .tm_min  = min,
        .tm_sec  = sec,
        .tm_nsec = 0,
    };
    return rtc_set_time(r->dev, &t);
}

int watch_rtc_get(WatchRTC *r, struct rtc_time *t)
{
    return rtc_get_time(r->dev, t);
}

void watch_rtc_print(const struct rtc_time *t)
{
    printf("RTC: %04d-%02d-%02d %02d:%02d:%02d\n",
           t->tm_year + 1900,
           t->tm_mon  + 1,
           t->tm_mday,
           t->tm_hour,
           t->tm_min,
           t->tm_sec);
}
