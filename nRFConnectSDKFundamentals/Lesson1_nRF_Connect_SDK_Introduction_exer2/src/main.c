/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   100

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_NODELABEL(led0)
#define LED1_NODE DT_NODELABEL(led1)
#define LED2_NODE DT_NODELABEL(led2)
#define LED3_NODE DT_NODELABEL(led3)

//Tạo object					node identifier, nơi chứa thông tin cần lấy
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

void main(void)
{
	int ret;
						//device pointer
	if (!device_is_ready(led0.port)) {
		return;
	}

		//Set chân GPIO này thành dạng output 
		//ACTIVE - Set High và logical = 1
		//INACTIVE - Set Low và logical = 0
		//INIT_LOW - Set Low
		//INIT_HIGH - Set High
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);

	if (ret < 0) {
		return;
	}

	while (1) {
			//đổi trạng thái của chân GPIO
		ret = gpio_pin_toggle_dt(&led0);
		gpio_pin_toggle_dt(&led1);
		gpio_pin_toggle_dt(&led2);
		gpio_pin_toggle_dt(&led3);
		if (ret < 0) {
			return;
		}
		k_msleep(SLEEP_TIME_MS);
	}
}
