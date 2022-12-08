/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 * Note:
 * Tested on nRF Connect SDK Version : 2.0
 */

#include <zephyr.h>
#include <drivers/gpio.h>

/* STEP 9 - Increase the sleep time from 100ms to 10 minutes  */
#define SLEEP_TIME_MS 10*60*1000

/* SW0_NODE is the devicetree node identifier for the "sw0" alias */
#define SW0_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

/* LED0_NODE is the devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* STEP 4 - Tạo hàm callback (hàm sẽ được gọi khi có interrupt) */
void button_presed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	gpio_pin_toggle_dt(&led);
}


/* STEP 5 - Define a variable of type static struct gpio_callback */
//Khai báo obj để chứa dữ liệu callback
static struct gpio_callback button_cb_data;

void main(void)
{
	int ret;
	if (!device_is_ready(led.port))
	{
		return;
	}

	if (!device_is_ready(button.port))
	{
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0)
	{
		return;
	}
	/* STEP 3 - Tạo interrupt vào obj button */
	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	/* STEP 6 - Khởi tạo callback vào obj vừa tạo   */
						//tên obj		hàm sẽ gọi khi có interrupt
	gpio_init_callback(&button_cb_data, button_presed, BIT(button.pin));
	/* STEP 7 - Thêm callback vào system bằng hàm gpio_add_callback()   */
					 //cổng GPIO nào, obj callback nào
	gpio_add_callback(button.port, &button_cb_data);
	while (1)
	{
		/* STEP 8 - Remove the polling code */
		

		k_msleep(SLEEP_TIME_MS);
	}
}