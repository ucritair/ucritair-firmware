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
#include <zephyr/drivers/gpio.h>

#include <zephyr/shell/shell.h>

#include <zephyr/sys/reboot.h>
#include <zephyr/sys/poweroff.h>

#include "lcd_driver.h"
#include "lcd_rendering.h"
#include "ble.h"
#include "wlan.h"
#include "sdcard.h"
#include "flash.h"
#include "rgb_leds.h"
#include "rtc.h"
#include "airquality.h"

static int cmd_start_wifi(const struct shell *sh, size_t argc,
				   char **argv)
{
#ifdef CONFIG_WIFI
	const struct device *wifi_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_wifi));

	if (device_init(wifi_dev))
	{
		printk("failed to init wlan");
	}

	set_mac();
#else
	LOG_ERR("Wifi compiled out");
#endif

	return 0;
}


SHELL_STATIC_SUBCMD_SET_CREATE(sub_x,
	SHELL_CMD_ARG(wifi_start, NULL,
		  "Start wifi",
		  cmd_start_wifi, 1, 0),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(x, &sub_x, "Log test", NULL);

int main(void)
{
	// while (1) {
	// 	k_msleep(1000);
	// }

	init_power_control();
	check_rtc_init();

	LOG_INF("~Test speaker~");
	set_3v3(true);
	

	usb_enable(NULL);

	const struct device* dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
	uint32_t dtr = 0;
	while ((!dtr) && (k_uptime_get() < 1500)) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		LOG_INF("Waiting for DTR...");
		k_sleep(K_MSEC(1000));
	}

	LOG_INF("CAT Application Started");
	LOG_PANIC();

	NRF_CLOCK_S->HFCLKCTRL = (CLOCK_HFCLKCTRL_HCLK_Div1 << CLOCK_HFCLKCTRL_HCLK_Pos); // 128MHz
	// Annoyingly need to do this even in low power because the LED timing depends on it

	init_buttons();

	if (wakeup_is_from_timer)
	{
		LOG_DBG("Booted from timer");
		set_5v0(true);
		set_leds(true);

		set_first_led((struct led_rgb){0, 100, 0});

		sensor_init();

		int cycle = 1;
		while (cycle++)
		{
			set_first_led((struct led_rgb){0, ((cycle%20)>10)?50:100, 0});
			k_msleep(20);

			sensor_read_once();
			if (get_buttons())
			{
				snapshot_rtc_for_reboot();
				wakeup_is_from_timer = false;
				sys_reboot(SYS_REBOOT_WARM);
			}

			// ~15s for PM
			// ~65s for NOC+VOX!?!?!

			if (current_readings.lps22hh.uptime_last_updated &&
				current_readings.sunrise.uptime_last_updated &&
				current_readings.sen5x.uptime_last_updated //&&
				// current_readings.sen5x.voc_index &&
				// current_readings.sen5x.nox_index
				)
			{
				LOG_INF("readings ready");
				epaper_render_test();
				k_msleep(20);
				power_off(sensor_wakeup_rate*1000);
			}
		}
	}

	test_speaker();

	ble_main();

	LOG_INF("Config Q2/Q3");
	if (gpio_pin_configure(DEVICE_DT_GET(DT_CHOSEN(gpio0)), 15, GPIO_OUTPUT_HIGH)) LOG_ERR("Init 0.15 failed");
	gpio_pin_set(DEVICE_DT_GET(DT_CHOSEN(gpio0)), 15, true);
	if (gpio_pin_configure(DEVICE_DT_GET(DT_CHOSEN(gpio0)), 16, GPIO_OUTPUT_HIGH)) LOG_ERR("Init 0.16 failed");
	gpio_pin_set(DEVICE_DT_GET(DT_CHOSEN(gpio0)), 16, true);

	const struct device *flash_dev = DEVICE_DT_GET_ONE(jedec_spi_nor);

	if (device_init(flash_dev))
	{
		printk("failed to init flash_dev");
	}

	test_flash();
	
	imu_init();

	const struct device *sdhc_dev = DEVICE_DT_GET(DT_CHOSEN(xxx_sdhcd0));

	if (device_init(sdhc_dev))
	{
		printk("failed to init sdhc");
	}

	const struct device *mmc_dev = DEVICE_DT_GET(DT_CHOSEN(xxx_mmc));

	if (device_init(mmc_dev))
	{
		printk("failed to init mmc");
	}

	test_sdcard();

	cmd_start_wifi(NULL, 0, NULL);

	set_5v0(true);
	k_msleep(50);

	sensor_init();

	lcd_init();
	lcd_render_diag();
}
