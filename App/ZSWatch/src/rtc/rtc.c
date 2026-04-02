#include "rtc.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(watch_rtc, LOG_LEVEL_INF);

static int watch_rtc_month_from_build_date(const char *month)
{
    static const char *const months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };
    size_t i;

    for (i = 0; i < ARRAY_SIZE(months); ++i) {
        if (strncmp(month, months[i], 3) == 0) {
            return (int)i + 1;
        }
    }

    return -EINVAL;
}

static int watch_rtc_parse_two_digits(const char *text)
{
    if ((text[0] < '0') || (text[0] > '9') || (text[1] < '0') || (text[1] > '9')) {
        return -EINVAL;
    }

    return ((text[0] - '0') * 10) + (text[1] - '0');
}

static int watch_rtc_parse_day(const char *text)
{
    int tens;
    int ones;

    if (((text[0] != ' ') && ((text[0] < '0') || (text[0] > '9'))) ||
        (text[1] < '0') || (text[1] > '9')) {
        return -EINVAL;
    }

    tens = (text[0] == ' ') ? 0 : (text[0] - '0');
    ones = text[1] - '0';
    return (tens * 10) + ones;
}

static int watch_rtc_day_of_week(int year, int month, int day)
{
    static const int offsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

    if (month < 3) {
        year -= 1;
    }

    return (year + year / 4 - year / 100 + year / 400 + offsets[month - 1] + day) % 7;
}

static int watch_rtc_seed_from_build_time(WatchRTC *r)
{
    const char *build_date = __DATE__;
    const char *build_time = __TIME__;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int err;

    if (r == NULL) {
        return -EINVAL;
    }

    month = watch_rtc_month_from_build_date(build_date);
    day = watch_rtc_parse_day(&build_date[4]);
    year = ((build_date[7] - '0') * 1000) +
           ((build_date[8] - '0') * 100) +
           ((build_date[9] - '0') * 10) +
           (build_date[10] - '0');
    hour = watch_rtc_parse_two_digits(&build_time[0]);
    minute = watch_rtc_parse_two_digits(&build_time[3]);
    second = watch_rtc_parse_two_digits(&build_time[6]);

    if ((month < 0) || (day < 0) || (hour < 0) || (minute < 0) || (second < 0)) {
        LOG_ERR("Failed to parse firmware build timestamp");
        return -EINVAL;
    }

    err = watch_rtc_set(r, year, month, day, hour, minute, second);
    if (err != 0) {
        LOG_ERR("Failed to seed RTC from build timestamp (err %d)", err);
        return err;
    }

    LOG_WRN("RTC had no valid time, seeded from firmware build timestamp %s %s",
            build_date, build_time);
    return 0;
}

int watch_rtc_init(WatchRTC *r)
{
    struct rtc_time t;
    int err;

    if (r == NULL) {
        return -EINVAL;
    }

    r->dev = DEVICE_DT_GET(DT_NODELABEL(rv8263));
    if (!device_is_ready(r->dev)) {
        return -ENODEV;
    }

    err = rtc_get_time(r->dev, &t);
    if (err == -ENODATA) {
        return watch_rtc_seed_from_build_time(r);
    }

    if (err != 0) {
        LOG_ERR("Failed to read RTC during init (err %d)", err);
        return err;
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
        .tm_wday = watch_rtc_day_of_week(year, month, day),
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
