/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 * Note:
 * Tested on nRF Connect SDK Version : 2.0
 */

/* Controlling LEDs through UART. Press 1-3 on your keyboard to toggle LEDS 1-3 on your development kit */

#include <zephyr.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <logging/log.h>

/* STEP 3 - Include the header file of the UART driver in main.c */
#include <drivers/uart.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 100000

/* STEP 10.1.1 - Define the size of the receive buffer */
#define RECEIVE_BUFF_SIZE 2
/* STEP 10.2 - Define the receiving timeout period */
#define RECEIVE_TIMEOUT 3000000
/* STEP 5.1 - Get the device pointers of the LEDs through gpio_dt_spec */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
/* STEP 4.1 - Get the device pointer of the UART hardware */
const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

/* STEP 9.1 - Define the transmission buffer, which is a buffer to hold the data to be sent over UART */
static uint8_t tx_buf[] = {"nRF Connect SDK Fundamentals Course\n\r"
						   "Press 1-3 on your keyboard to toggle LEDS 1-3 on your development kit\n\r"};
/* STEP 10.1.2 - Define the receive buffer */
static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};

LOG_MODULE_REGISTER(Less5_Exer1, LOG_LEVEL_DBG);

/* STEP 7 - Define the callback function for UART */

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	// Truy cập prop type của evt
	switch (evt->type)
	{
		// Khi nhận được data
	case UART_RX_RDY:
		
		printk("%s \n", evt->data.rx.buf);

		// Muốn truyền vào bao nhiêu thì set size buffer bấy nhiêu không set thừa vì offset sẽ bị nhảy

		//Tùy vào trường hợp mà có giải thuật tương ứng vì đôi khi sẽ có lỗi liên quan đến offset
		//Nếu truyền dữ liệu vào từ bàn phím
		//  truy cập prop len của prop rx của prop data
		if ((evt->data.rx.offset) >= 1)
		{
			if (strcmp(evt->data.rx.buf,"11") == 0)
				gpio_pin_toggle_dt(&led0);
			else if (strcmp(evt->data.rx.buf, "22") == 0)
				gpio_pin_toggle_dt(&led1);
			else if (strcmp(evt->data.rx.buf, "33") == 0)
				gpio_pin_toggle_dt(&led2);
			// memset(evt->data.rx.buf, 0, 2);
		}
		

		break;
		// Khi nhận xong data hoặc timeout
	case UART_RX_DISABLED:
		uart_rx_enable(dev, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);

		break;
	case UART_TX_DONE:
		printk("UART TX DONE\r\n");
		break;

	default:
		break;
	}
}

int main(void)
{
	int ret;
	{
		/* STEP 4.2 - Verify that the UART device is ready */
		if (!device_is_ready(uart))
		{
			printk("UART device not ready\r\n");
			return 1;
		}
		/* STEP 5.2 - Verify that the LED devices are ready */
		if (!device_is_ready(led0.port))
		{
			printk("GPIO device is not ready\r\n");
			return 1;
		}
		/* STEP 6 - Configure the GPIOs of the LEDs */

		ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
		if (ret < 0)
		{
			return 1;
		}
		ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
		if (ret < 0)
		{
			return 1;
		}
		ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
		if (ret < 0)
		{
			return 1;
		}
	}
	/* STEP 8 - Tạo hàm callback gán vào chân uart */
	//  					chân  tên hàm gán
	ret = uart_callback_set(uart, uart_cb, NULL);
	if (ret)
	{
		return 1;
	}
	/* STEP 9.2 - Send the data over UART by calling uart_tx() */
	ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
	if (ret)
	{
		return 1;
	}
	/* STEP 10.3  - Bắt đầu nhận data = hàm uart_rx_enable() */
	// 					 chân  nơi chứa data nhận được, size, thời gian chờ tối đa kể từ lúc nhận đc data trước 
	//Nếu timeout thì cũng gọi interrupt như khi buffer đầy (tính trong microsec μs)
	//Nếu để -1 thì chỉ interrupt khi đầy buffer và offset tự động set về 0
	//buffer đầy khi truyền đủ tự động ngắt khác OFFSET với được truyền dần vào (do timeout) 
	ret = uart_rx_enable(uart, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
	if (ret)
	{
		return 1;
	}
	while (1)
	{
		k_msleep(SLEEP_TIME_MS);
	}
}