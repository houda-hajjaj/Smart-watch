#include "input/buttons.h"

#include <errno.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

#include "thread/power_thread.h"

LOG_MODULE_REGISTER(buttons, LOG_LEVEL_INF);

#define BUTTON_DEBOUNCE_MS 150

static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
static const struct gpio_dt_spec sw1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw1), gpios, {0});

static struct gpio_callback sw0_cb_data;
static struct gpio_callback sw1_cb_data;
static atomic_t button_events;
static int64_t last_sw0_press_ms;
static int64_t last_sw1_press_ms;
static bool buttons_initialized;

static void buttons_sw0_pressed(const struct device *port,
                                struct gpio_callback *cb,
                                gpio_port_pins_t pins)
{
    int64_t now_ms;

    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    now_ms = k_uptime_get();
    if ((now_ms - last_sw0_press_ms) < BUTTON_DEBOUNCE_MS) {
        return;
    }

    last_sw0_press_ms = now_ms;
    if (power_thread_is_eco_mode()) {
        return;
    }

    (void)atomic_or(&button_events, BUTTON_EVENT_NEXT_SCENE);
}

static void buttons_sw1_pressed(const struct device *port,
                                struct gpio_callback *cb,
                                gpio_port_pins_t pins)
{
    int64_t now_ms;

    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    now_ms = k_uptime_get();
    if ((now_ms - last_sw1_press_ms) < BUTTON_DEBOUNCE_MS) {
        return;
    }

    last_sw1_press_ms = now_ms;
    if (!power_thread_is_eco_mode()) {
        (void)atomic_or(&button_events, BUTTON_EVENT_WAKE);
    }
    power_thread_notify_activity();
}

static int buttons_configure_one(const struct gpio_dt_spec *spec,
                                 struct gpio_callback *callback,
                                 gpio_callback_handler_t handler,
                                 const char *name)
{
    int err;

    if (spec->port == NULL) {
        LOG_WRN("%s alias not available", name);
        return -ENODEV;
    }

    if (!device_is_ready(spec->port)) {
        LOG_WRN("%s GPIO device not ready", name);
        return -ENODEV;
    }

    err = gpio_pin_configure_dt(spec, GPIO_INPUT);
    if (err != 0) {
        LOG_ERR("%s configure failed (err %d)", name, err);
        return err;
    }

    err = gpio_pin_interrupt_configure_dt(spec, GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        LOG_ERR("%s interrupt config failed (err %d)", name, err);
        return err;
    }

    gpio_init_callback(callback, handler, BIT(spec->pin));
    err = gpio_add_callback(spec->port, callback);
    if (err != 0) {
        LOG_ERR("%s callback add failed (err %d)", name, err);
        return err;
    }

    LOG_INF("%s configured on pin %d", name, spec->pin);
    return 0;
}

int buttons_init(void)
{
    int sw0_err;
    int sw1_err;

    if (buttons_initialized) {
        return 0;
    }

    sw0_err = buttons_configure_one(&sw0, &sw0_cb_data, buttons_sw0_pressed, "sw0");
    sw1_err = buttons_configure_one(&sw1, &sw1_cb_data, buttons_sw1_pressed, "sw1");

    if ((sw0_err != 0) && (sw1_err != 0)) {
        return sw0_err;
    }

    buttons_initialized = true;
    return 0;
}

uint32_t buttons_consume_events(void)
{
    return (uint32_t)atomic_set(&button_events, BUTTON_EVENT_NONE);
}
