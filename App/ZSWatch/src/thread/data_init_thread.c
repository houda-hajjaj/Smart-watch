#include "thread/data_init_thread.h"

#include <string.h>

#include <zephyr/logging/log.h>

#include "ble/ble_service.h"

LOG_MODULE_REGISTER(data_init_thread, LOG_LEVEL_INF);

#define DATA_INIT_THREAD_STACK_SIZE 2048
#define DATA_INIT_THREAD_PRIORITY 1

K_THREAD_STACK_DEFINE(data_init_stack, DATA_INIT_THREAD_STACK_SIZE);
static struct k_thread data_init_thread;
static k_tid_t data_init_tid;

K_SEM_DEFINE(data_init_done_sem, 0, 1);

static DataInitContext data_ctx;
static bool data_init_started;

static void data_init_entry(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    memset(&data_ctx, 0, sizeof(data_ctx));

    data_ctx.motion_ready = (motion_init(&data_ctx.motion) == 0);
    if (!data_ctx.motion_ready) {
        LOG_ERR("motion_init failed");
    }

    data_ctx.env_ready = (env_init(&data_ctx.env) == 0);
    if (!data_ctx.env_ready) {
        LOG_ERR("env_init failed");
    }

    data_ctx.mag_ready = (mag_init(&data_ctx.mag) == 0);
    if (!data_ctx.mag_ready) {
        LOG_ERR("mag_init failed");
    }

    data_ctx.rtc_ready = (watch_rtc_init(&data_ctx.rtc) == 0);
    if (!data_ctx.rtc_ready) {
        LOG_ERR("watch_rtc_init failed");
    }

    data_ctx.ble_ready = (ble_service_init() == 0);
    if (!data_ctx.ble_ready) {
        LOG_ERR("ble_service_init failed");
    }

    data_ctx.initialized = data_ctx.motion_ready &&
                           data_ctx.env_ready &&
                           data_ctx.mag_ready &&
                           data_ctx.rtc_ready &&
                           data_ctx.ble_ready;

    if (data_ctx.initialized) {
        LOG_INF("Data initialization completed");
    } else {
        LOG_WRN("Data initialization completed with errors");
    }

    k_sem_give(&data_init_done_sem);
}

void data_init_thread_start(void)
{
    if (data_init_started) {
        return;
    }

    data_init_started = true;
    data_init_tid = k_thread_create(&data_init_thread,
                                    data_init_stack,
                                    K_THREAD_STACK_SIZEOF(data_init_stack),
                                    data_init_entry,
                                    NULL,
                                    NULL,
                                    NULL,
                                    DATA_INIT_THREAD_PRIORITY,
                                    0,
                                    K_NO_WAIT);
    k_thread_name_set(data_init_tid, "data_init");
}

bool data_init_thread_wait_done(k_timeout_t timeout)
{
    if (!data_init_started) {
        return false;
    }

    if (k_sem_take(&data_init_done_sem, timeout) != 0) {
        return false;
    }

    k_sem_give(&data_init_done_sem);
    return true;
}

bool data_init_thread_wait_ready(k_timeout_t timeout)
{
    if (!data_init_thread_wait_done(timeout)) {
        return false;
    }

    return data_ctx.initialized;
}

bool data_init_thread_is_ready(void)
{
    return data_ctx.initialized;
}

DataInitContext *data_init_thread_get_context_mutable(void)
{
    return &data_ctx;
}

const DataInitContext *data_init_thread_get_context(void)
{
    return &data_ctx;
}
