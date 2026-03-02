/*
 * Copyright (c) 2024 LVGL <felipe@lvgl.io>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <lvgl_mem.h>
//#include <lv_demos.h>
#include "ui/ui.h"
#include "sensor/env_sensor.h"
#include <stdio.h>
#include <stdlib.h>

EnvSensor g_sensor;

static void sensor_update_cb(lv_timer_t *timer)
{
	char buf[16];
	if (env_update(&g_sensor) == 0) {
		snprintf(buf, sizeof(buf), "%d.%d",
			g_sensor.temp_hts.val1,
			abs(g_sensor.temp_hts.val2) / 100000);
		lv_label_set_text(ui_Label2, buf);
	}
}

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

int main(void)
{
	const struct device *display_dev;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

	/* Place your UI init function here */
	ui_init();

	if (env_init(&g_sensor) == 0) {
		lv_timer_create(sensor_update_cb, 1000, NULL);
	} else {
		LOG_ERR("Sensor init failed");
	}

	display_blanking_off(display_dev);
#ifdef CONFIG_LV_Z_MEM_POOL_SYS_HEAP
	lvgl_print_heap_info(false);
#else
	printf("lvgl in malloc mode\n");
#endif
	while (1) {
	        //This function has to be called periodically to update the UI
	        //Can be placed in a dedicated thread
		uint32_t sleep_ms = lv_timer_handler();
		k_msleep(MIN(sleep_ms, INT32_MAX));
	}

	return 0;
}
