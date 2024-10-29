/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sample, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>

#include <zephyr/logging/log_ctrl.h>

#include "lcd_driver.h"
#include "lcd_rendering.h"
#include "ble.h"
#include "wlan.h"

int main(void)
{
	init_power_control();

	LOG_INF("~Test speaker~");
	set_3v3(true);
	test_speaker();

	usb_enable(NULL);

	// uint32_t dtr = 0;
	// while (!dtr) {
	// 	uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
	// 	LOG_INF("Waiting for DTR...");
	// 	k_sleep(K_MSEC(1000));
	// }

	LOG_INF("CAT Application Started");
	LOG_PANIC();

	init_buttons();

	ble_main();
	set_mac();

	set_5v0(true);
	k_msleep(50);

	sensor_init();

	lcd_init();
	lcd_render_diag();
}
