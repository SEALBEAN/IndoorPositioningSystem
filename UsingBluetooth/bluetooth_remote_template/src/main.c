
/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>
#include "remote.h"

#define LOG_MODULE_NAME app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define RUN_STATUS_LED DK_LED1
#define CONN_STATUS_LED DK_LED2
#define RUN_LED_BLINK_INTERVAL 1000

/* Declarations */

void on_connected(struct bt_conn *conn, uint8_t err);
void on_disconnected(struct bt_conn *conn, uint8_t reason);
void on_notif_changed(enum bt_button_notifications_enabled status);
void on_data_received(struct bt_conn *conn, const uint8_t *const data, uint16_t len);

/* Callbacks */

static struct bt_conn *current_conn;

struct bt_conn_cb bluetooth_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
};

struct bt_remote_service_cb remote_callbacks = {
	.notif_changed = on_notif_changed,
	.data_received = on_data_received,
};

/*
	Call backs được chạy khi nhận được dữ liệu
*/
void on_data_received(struct bt_conn *conn, const uint8_t *const data, uint16_t len)
{
	// Tạo String để lưu dữ liệu và chốt kí tự cuối là /0 để thông báo kết thúc String
	uint8_t temp_str[len + 1];
	memcpy(temp_str, data, len);
	temp_str[len] = 0x00;

	LOG_INF("Received data on conn %p. Len: %d", (void *)conn, len);
	LOG_INF("Data: %s", temp_str);
}

/*
	Call backs được chạy khi Central bật chế độ nhận Notify
*/
void on_notif_changed(enum bt_button_notifications_enabled status)
{
	if (status == BT_BUTTON_NOTIFICATIONS_ENABLED)
	{
		LOG_INF("Notifications enabled");
	}
	else
	{
		LOG_INF("Notificatons disabled");
	}
}

/*
	Call backs được chạy khi đã kết nối với central
*/
void on_connected(struct bt_conn *conn, uint8_t err)
{
	if (err)
	{
		LOG_ERR("connection err: %d", err);
		return;
	}
	LOG_INF("Connected.");
	// Tăng biến đếm số lượng connect
	current_conn = bt_conn_ref(conn);
	dk_set_led_on(CONN_STATUS_LED);
}

/*
	Call backs được chạy khi ngắt kết nối với central
*/
void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason: %d)", reason);
	dk_set_led_off(CONN_STATUS_LED);
	if (current_conn)
	{
		// Giảm biến đếm số lượng connect
		bt_conn_unref(current_conn);
		// Nếu số lượng = 0 thì nó là NULL
		// Ở đây chắc muốn chỉ kết nối được 1 thiết bị nên gán lại cho chắc
		current_conn = NULL;
	}
}

void button_handler(uint32_t button_state, uint32_t has_changed)
{
	int button_pressed = 0;
	// Giá trị của button_state và has_changed được các hàm khác truyền vào
	// tương ứng với số bit của đèn tương ứng 0001  0010   0100  1000
	if (has_changed & button_state)
	{
		switch (has_changed)
		{
		case DK_BTN1_MSK:
			button_pressed = 1;
			break;
		case DK_BTN2_MSK:
			button_pressed = 2;
			break;
		case DK_BTN3_MSK:
			button_pressed = 3;
			break;
		case DK_BTN4_MSK:
			button_pressed = 4;
			break;
		default:
			break;
		}
		LOG_INF("Button %d pressed.", button_pressed);
		set_button_value(button_pressed);
		// Gọi hàm bên remote.c để in thông báo ra console
		// Lưu ý data trên app đọc được sẽ bị ngược chiều
		// Dữ liệu gửi	số byte
		int err = send_button_notification(current_conn, button_pressed, 2);
		if (err)
		{
			LOG_ERR("couldn't send notification (err: %d)", err);
		}
	}
}

/* Configurations */

static void configure_dk_buttons_leds(void)
{
	int err;

	// Thiết lập callbacks sẽ được thực hiện khi giá trị của button thay đổi
	err = dk_buttons_init(button_handler);
	if (err)
	{
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}
	err = dk_leds_init();
	if (err)
	{
		LOG_ERR("Cannot init LEDs (err: %d)", err);
	}
}

/* main */

void main(void)
{
	int err;
	int blink_status = 0;
	LOG_INF("Hello World! %s\n", CONFIG_BOARD);

	configure_dk_buttons_leds();

	err = bluetooth_init(&bluetooth_callbacks, &remote_callbacks);
	if (err)
	{
		LOG_ERR("bt_enable returned %d", err);
	}

	LOG_INF("Running...");
	for (;;)
	{
		dk_set_led(RUN_STATUS_LED, (blink_status++ % 2));
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}
