/**
 * Copyright (c) 2025 Hervé Boeglen
 * SPDX-License-Identifier: Apache-2.0
 */
#define DT_DRV_COMPAT ti_tsc2007

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/input/input.h>
#include <zephyr/input/input_touch.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(tsc2007, CONFIG_INPUT_LOG_LEVEL);

#include <stdlib.h>

#define TSC2007_MEASURE_TEMP0	        (0x0 << 4)
#define TSC2007_MEASURE_AUX		(0x2 << 4)
#define TSC2007_MEASURE_TEMP1	        (0x4 << 4)
#define TSC2007_ACTIVATE_XN		(0x8 << 4)
#define TSC2007_ACTIVATE_YN		(0x9 << 4)
#define TSC2007_ACTIVATE_YP_XN	        (0xa << 4)
#define TSC2007_SETUP			(0xb << 4)
#define TSC2007_MEASURE_X		(0xc << 4)
#define TSC2007_MEASURE_Y		(0xd << 4)
#define TSC2007_MEASURE_Z1		(0xe << 4)
#define TSC2007_MEASURE_Z2		(0xf << 4)

#define TSC2007_POWER_OFF_IRQ_EN	(0x0 << 2)
#define TSC2007_ADC_ON_IRQ_DIS0		(0x1 << 2)
#define TSC2007_ADC_OFF_IRQ_EN		(0x2 << 2)
#define TSC2007_ADC_ON_IRQ_DIS1		(0x3 << 2)

#define TSC2007_12BIT			(0x0 << 1)
#define TSC2007_8BIT			(0x1 << 1)

#define	MAX_12BIT			((1 << 12) - 1)

#define ADC_ON_12BIT	(TSC2007_12BIT | TSC2007_ADC_ON_IRQ_DIS0)

#define READ_Y		(ADC_ON_12BIT | TSC2007_MEASURE_Y)
#define READ_Z1		(ADC_ON_12BIT | TSC2007_MEASURE_Z1)
#define READ_Z2		(ADC_ON_12BIT | TSC2007_MEASURE_Z2)
#define READ_X		(ADC_ON_12BIT | TSC2007_MEASURE_X)
#define PWRDOWN		(TSC2007_12BIT | TSC2007_POWER_OFF_IRQ_EN)

#define RX_PLATE 335 //measured resistance between X+ and X- terminals

#define RESOLUTION_12BITS 4096
#define SLEEP_TIME_US	600
#define TSC_DELAY 1

struct tsc2007_config {
	struct input_touchscreen_common_config common;
	struct i2c_dt_spec bus;
	struct gpio_dt_spec int_gpio;
	int raw_x_min;
	int raw_y_min;
	uint16_t raw_x_max;
	uint16_t raw_y_max;
	bool inverted_x;
	bool inverted_y;
};

struct tsc2007_data {
	const struct device *dev;
	struct k_work processing_work;
	struct gpio_callback int_gpio_cb;
	uint32_t touch_x;
	uint32_t touch_y;
	uint32_t touch_press;
};

INPUT_TOUCH_STRUCT_CHECK(struct tsc2007_config);

static int tsc2007_command(const struct i2c_dt_spec i2c_dev, uint8_t command)
{
	int ret;
    uint8_t data[2];

    uint8_t com[1] = {command};

    ret=i2c_write_dt(&i2c_dev, com, 1);

    k_usleep(SLEEP_TIME_US);

    ret=i2c_read_dt(&i2c_dev,data,2);
    
	uint16_t valc = (data[0] << 4) | (data[1] >> 4);

	return valc;
}

static uint32_t tsc2007_calc_pressure(const struct i2c_dt_spec i2c_dev, uint32_t *x)
{
	int z1, z2;
	uint32_t press = 0;

	z1 = tsc2007_command(i2c_dev, READ_Z1);

	z2 = tsc2007_command(i2c_dev, READ_Z2);

	// This is the implementation of Equ. (1) in the datasheet (page 13)
	press = z2 - z1;
	press *= (uint32_t)&x;
	press *= (uint32_t)RX_PLATE;
	press /= z1;
	press = (press + 2047) >> 12;

	return press;
}

static int tsc2007_get_data(const struct device *dev)
{
	const struct tsc2007_config *config = dev->config;
	struct tsc2007_data *data = dev->data;
	
	int x1, x2, y1, y2;

  // take two measurements since there can be a 'flicker' on pen up
  x1 = tsc2007_command(config->bus, READ_X);
  y1 = tsc2007_command(config->bus, READ_Y);
  x2 = tsc2007_command(config->bus, READ_X);
  y2 = tsc2007_command(config->bus, READ_Y);

  if (abs((int32_t)x1 - (int32_t)x2) > 100)
    return -1;
  if (abs((int32_t)y1 - (int32_t)y2) > 100)
    return -1;

  data->touch_x = x1;
  data->touch_y = y1;

  data->touch_press = tsc2007_calc_pressure(config->bus, &data->touch_x);
  
  return (data->touch_x < 4095) && (data->touch_y < 4095);
}

static void tsc2007_report_touch(const struct device *dev)
{
	const struct tsc2007_config *config = dev->config;
	const struct input_touchscreen_common_config *common = &config->common;
	struct tsc2007_data *data = dev->data;
	int x = data->touch_x;
	int y = data->touch_y;

	printk("Raw x = %d, y = %d\n",x, y);

	if (common->screen_width > 0 && common->screen_height > 0) {
		x = data->touch_x;
		y = data->touch_y;

		//if (common->inverted_x) {
		//	data->touch_x = common->screen_width - data->touch_x;
		//}
		//if (common->inverted_y) {
		//	data->touch_y = common->screen_height - data->touch_y;
		//}

		//printk("x = %d, y = %d\n",x, y);

		x = CLAMP(x, 0, common->screen_width);
		y = CLAMP(y, 0, common->screen_height);
		//x = 213;
		//y = 242;
	}

	input_touchscreen_report_pos(dev, x, y, K_FOREVER);
	input_report_key(dev, INPUT_BTN_TOUCH, 1, true, K_FOREVER);
	k_msleep(50);

	input_report_key(dev, INPUT_BTN_TOUCH, 0, true, K_FOREVER);
}

static int tsc2007_process(const struct device *dev)
{
	const struct tsc2007_config *config = dev->config;

	int err;
	uint8_t command[1] = {PWRDOWN};
	
	//gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_DISABLE);

	err = tsc2007_get_data(dev);
	if (err < 0) {
		return err;
	}
    
	//printk("Touch detected!\n");
	tsc2007_report_touch(dev);

	// int val = gpio_pin_get_dt(&config->int_gpio);
	// printk("Pin value = %d\n",val);
	
	/* Re-enable PENIRQ\ */
	  err = i2c_write_dt(&config->bus, command, sizeof(command));
	  if (err < 0) {
	  	LOG_ERR("Could not initialize TSC2007 (%d)", err);
	  	return err;
	  }
	k_usleep(SLEEP_TIME_US);
	//gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_EDGE_FALLING);
	
	return 0;
}

static void tsc2007_work_handler(struct k_work *work)
{
	struct tsc2007_data *data = CONTAINER_OF(work, struct tsc2007_data, processing_work);
	const struct tsc2007_config *config = data->dev->config;

	gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_DISABLE);
	//gpio_remove_callback(config->int_gpio.port, &data->int_gpio_cb);
	
	tsc2007_process(data->dev);
	k_msleep(10);
	
	gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_EDGE_FALLING);
	//gpio_add_callback(config->int_gpio.port, &data->int_gpio_cb);
	/**
	 * Reschedule ISR if there was an interrupt triggered during handling (race condition).
	 * IRQ is edge-triggered, so otherwise it would never be triggered again.
	 */
	if (gpio_pin_get_dt(&config->int_gpio)) {
		k_work_submit(&data->processing_work);
	}
}

static void tsc2007_interrupt_handler(const struct device *dev, struct gpio_callback *cb,
				       uint32_t pins)
{
	//const struct tsc2007_config *config = dev->config;
	struct tsc2007_data *data = CONTAINER_OF(cb, struct tsc2007_data, int_gpio_cb);
	//printk("Interrupt occured!\n");

	k_work_submit(&data->processing_work);
}

// static int tsc2007_init(const struct device *dev)
// {
// 	printk("Hello!\n");

// 	return 0;
// }
static int tsc2007_init(const struct device *dev)
{
	const struct tsc2007_config *config = dev->config;
	struct tsc2007_data *data = dev->data;
	int err;

	if (!i2c_is_ready_dt(&config->bus)) {
		LOG_ERR("I2C controller device not ready");
		return -ENODEV;
	}
	else printk("TSC2007 init OK!\n");

	data->dev = dev;

	k_work_init(&data->processing_work, tsc2007_work_handler);

	/* Initialize GPIO interrupt */
	if (!gpio_is_ready_dt(&config->int_gpio)) {
		LOG_ERR("Interrupt GPIO controller device not ready");
		return -ENODEV;
	}

	err = gpio_pin_configure_dt(&config->int_gpio, GPIO_INPUT);
	if (err < 0) {
		LOG_ERR("Could not configure interrupt GPIO pin (%d)", err);
		return err;
	}

	err = gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_EDGE_FALLING);
	if (err < 0) {
		LOG_ERR("Could not configure GPIO interrupt (%d)", err);
		return err;
	}

	gpio_init_callback(&data->int_gpio_cb, tsc2007_interrupt_handler,
			   BIT(config->int_gpio.pin));
	err = gpio_add_callback_dt(&config->int_gpio, &data->int_gpio_cb);
	if (err < 0) {
		LOG_ERR("Could not set GPIO callback (%d)", err);
		return err;
	}

	/* 12 bit mode Enable PENIRQ */
	uint8_t command[1] = {PWRDOWN};
	err = i2c_write_dt(&config->bus, command, sizeof(command));
	if (err < 0) {
		LOG_ERR("Could not initialize TSC2007 (%d)", err);
		return err;
	}

	return 0;
}

#define TSC2007_DEFINE(index)                                                          \
										       \
   static struct tsc2007_data tsc2007_data_##index;                                    \
										       \
   static const struct tsc2007_config tsc2007_config_##index = {                       \
	.common = INPUT_TOUCH_DT_INST_COMMON_CONFIG_INIT(index),                       \
	.bus = I2C_DT_SPEC_INST_GET(index),                                            \
	.int_gpio = GPIO_DT_SPEC_INST_GET(index, int_gpios),                           \
	.raw_x_min = DT_INST_PROP_OR(index, raw_x_min, 0),                             \
	.raw_y_min = DT_INST_PROP_OR(index, raw_y_min, 0),                             \
	.raw_x_max = DT_INST_PROP_OR(index, raw_x_max, 4096),                          \
	.raw_y_max = DT_INST_PROP_OR(index, raw_y_max, 4096),						   \
	.inverted_x = DT_INST_PROP_OR(index, inverted_x, false),					   \
	.inverted_y = DT_INST_PROP_OR(index, inverted_y, false)};                      \
	                                                                               \
	DEVICE_DT_INST_DEFINE(index,                                                   \
			      tsc2007_init,                                            \
			      NULL,                                                    \
			      &tsc2007_data_##index,                                   \
			      &tsc2007_config_##index,                                 \
			      POST_KERNEL,                                             \
			      CONFIG_INPUT_INIT_PRIORITY,                              \
			      NULL);                                                   \

DT_INST_FOREACH_STATUS_OKAY(TSC2007_DEFINE)
