#include "thread/rtc_thread.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "thread/data_init_thread.h"
#include "rtc/rtc.h"

LOG_MODULE_REGISTER(rtc_thread, LOG_LEVEL_INF);

#define RTC_THREAD_STACK_SIZE 1536
#define RTC_THREAD_PRIORITY 9
#define RTC_THREAD_PERIOD_MS 1000

K_THREAD_STACK_DEFINE(rtc_thread_stack, RTC_THREAD_STACK_SIZE);
static struct k_thread rtc_thread_data;
static bool rtc_thread_started;

static void rtc_thread_entry(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    struct rtc_time now;

    if (!data_init_thread_wait_ready(K_FOREVER)) {
        LOG_ERR("RTC thread aborted: data init not ready");
        return;
    }

    DataInitContext *ctx = data_init_thread_get_context_mutable();
    LOG_INF("RTC thread started");

    while (1) {
        if (watch_rtc_get(&ctx->rtc, &now) == 0) {
            watch_rtc_print(&now);
        }

        k_sleep(K_MSEC(RTC_THREAD_PERIOD_MS));
    }
}

void rtc_thread_start(void)
{
    if (rtc_thread_started) {
        return;
    }

    rtc_thread_started = true;

    k_tid_t tid = k_thread_create(&rtc_thread_data,
                                  rtc_thread_stack,
                                  K_THREAD_STACK_SIZEOF(rtc_thread_stack),
                                  rtc_thread_entry,
                                  NULL,
                                  NULL,
                                  NULL,
                                  RTC_THREAD_PRIORITY,
                                  0,
                                  K_NO_WAIT);
    k_thread_name_set(tid, "rtc_thread");
}
