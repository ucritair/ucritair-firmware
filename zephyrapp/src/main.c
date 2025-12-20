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
#include "buttons.h"
#include "batt.h"
#include "rp2350_ipc.h"

int main(void)
{
	init_power_control();
	check_rtc_init();
	init_adc();

	// LOG_INF("~Test speaker~");
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

	init_epaper_enough_for_it_to_calm_the_fuck_down();

	NRF_CLOCK_S->HFCLKCTRL = (CLOCK_HFCLKCTRL_HCLK_Div1 << CLOCK_HFCLKCTRL_HCLK_Pos); // 128MHz
	// Annoyingly need to do this even in low power because the LED timing depends on it

	init_buttons();

	set_5v0(true);
	set_leds(true);

	sensor_init();
	imu_init();

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
	continue_rtc_from_log();

	if (wakeup_is_from_timer)
	{
		LOG_DBG("Booted from timer");

		set_first_led((struct led_rgb){.g=1});
		k_msleep(50);

		bool trying_to_take_nox_reading = nox_sample_period != 0 && (nox_sample_counter == (nox_sample_period-1));

		int cycle = 1;
#define CYCLE_TIME 20
		while (cycle++)
		{
			int intensity = ((cycle%20)>10)?1:3;
			struct led_rgb color = {0};
			if (trying_to_take_nox_reading)
			{
				color.b = intensity;
			}
			else
			{
				color.g = intensity;
			}

			set_first_led(color);
			k_msleep(CYCLE_TIME);

			sensor_read_once();
			update_buttons();

			if (current_buttons)
			{
				// snapshot_rtc_for_reboot();
				// wakeup_is_from_timer = false;
				// sys_reboot(SYS_REBOOT_WARM);
				break;
			}

			LOG_DBG("Waiting for sensors ready..., cycle=%d", cycle);

			// ~15s for PM
			// ~65s for NOC+VOX!?!?!

			bool are_ready = is_ready_for_aqi_logging();

			if (trying_to_take_nox_reading)
			{
				if ((readings.sen5x.voc_index == 0) ||
					(readings.sen5x.nox_index == 0))
				{
					are_ready = false;
				}
			}

			if (are_ready)
			{
				nox_sample_counter++;
				if (nox_sample_period == nox_sample_counter)
				{
					nox_sample_counter = 0;
				}

				last_sensor_timestamp = CAT_get_RTC_now();
				last_log_timestamp = CAT_get_RTC_now();

				LOG_INF("readings ready");
				k_msleep(20);
				populate_next_log_cell();
				LOG_INF("update eink");
				k_msleep(20);
				CAT_eink_draw_default();
				CAT_eink_update(false);
				LOG_INF("power off");
				k_msleep(20);
				power_off(sensor_wakeup_period*1000, false);
			}

			if (cycle > (120000/CYCLE_TIME))
			{
				// give up and try again later
				LOG_INF("Giving up and power off");
				power_off(sensor_wakeup_period*1000, false);
			}
		}
	}

	test_speaker();

	ble_main();

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

	lcd_init();
	lcd_render_diag();
}
