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
#include "msht.h"

#include "mt_test.h"

// memory monitoring
//
// System heap
extern struct sys_heap _system_heap;


#define PMSTAT \
ret = k_thread_stack_space_get(current_thread, &free_stack); \
if (ret < 0) { \
        printk("Failed to get stack stats\r\n"); \
} else { \
        printk("Stack | Free: %u bytes\r\n", free_stack); \
}

/*
\
ret = sys_heap_runtime_stats_get(&_system_heap, &heap_stats);\
if (ret < 0) { \
        printk("Failed to get heap stats\r\n"); \
} else { \
        printk("Heap  | Free: %u bytes, Allocated: %u bytes\r\n", \
        heap_stats.free_bytes, \
        heap_stats.allocated_bytes);\
} \
*/



int main(void)
{
	// more memory monitoring
	int ret;
	struct sys_memory_stats heap_stats;
	struct k_thread *current_thread = k_current_get();
	size_t free_stack;

	// while (1) {
	// 	k_msleep(1000);
	// }

	init_power_control();
	check_rtc_init();
	init_adc();

	// LOG_INF("~Test speaker~");
	set_3v3(true);

	usb_enable(NULL);
PMSTAT
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


// ---- MESHTASTIC BRINGUP DEBUG ----
PMSTAT
	msht_init();
PMSTAT


	/*
	while (1) {
		msht_w("[HENLO SERIAL FRIENDO!]\n");
		k_msleep(1000);
	}
	*/

	printk("the modem is eeby, let it take a bit to wake up...\r");
	k_msleep(12000);
	printk("ok.\n");

	// clear any protobufs we received on init
	if ( msht_status() )
	{
		msht_process(NULL);
	}
#if 0	
PMSTAT
	uint8_t tst_buf[] = {0x94,0xc3,0x00,0x06,0x18,0xa6,0xbe,0xb2,0xa3,0x0b};

	printk("SEND IT! %u\r\n", sizeof(tst_buf));

	msht_w(tst_buf, sizeof(tst_buf));

PMSTAT
#endif


	mt_send_text("Public hello, world!\n", BROADCAST_ADDR, 0);

	printk("TEST SENT!\r\n");
	
	k_msleep(1000);

	mt_send_text("Direct hello, world ^_^\n", 0x49f38538, 0);	

	// speeeeeen
	while (1) {
		if ( msht_status() )
		{
			msht_process(msht_test_callback);
		}

		k_msleep(100);
	}

// ---- NOTHING EXECD BELOW ----

	// Initialize RP2350 IPC
	LOG_INF("Initializing RP2350 IPC...\n");
	rp2350_ipc_init();
	k_msleep(10000); // Give RP2350 time to boot

	// Query firmware version
	uint8_t fw_major, fw_minor;
	uint16_t fw_patch;
	if (rp2350_query_firmware_version(&fw_major, &fw_minor, &fw_patch, 5000)) {
		LOG_INF("RP2350 Firmware: v%u.%u.%u\n", fw_major, fw_minor, fw_patch);
	} else {
		LOG_ERR("Failed to query RP2350 firmware version\n");
	}

	// Test reboot to bootloader
	/*k_msleep(2000); // Wait 2 seconds before rebooting
	printk("\n--- Testing RP2350 reboot to bootloader ---\n");
	if (rp2350_reboot_to_bootloader(2000)) {
		printk("Success! RP2350 should now appear as USB mass storage device.\n");
	} else {
		printk("Failed to reboot RP2350 to bootloader\n");
	}*/

	// Test WiFi scan
	/*LOG_INF("Requesting WiFi scan from RP2350 (may take 10-20 seconds)...\n");
	msg_payload_wifi_scan_response_t scan_results;
	if (rp2350_wifi_scan(&scan_results, 30000)){
		LOG_INF("WiFi scan successful! Found %d unique APs:\n", scan_results.count);
		for (int i = 0; i < scan_results.count; i++) {
			const char *auth_str[] = {"Open", "WEP", "WPA", "WPA2", "WPA/WPA2"};
			LOG_INF("  [%d] %s [%02X:%02X:%02X:%02X:%02X:%02X] RSSI: %d dBm, Ch: %d, Auth: %s\n",
			        i + 1,
			        scan_results.aps[i].ssid,
			        scan_results.aps[i].bssid[0], scan_results.aps[i].bssid[1],
			        scan_results.aps[i].bssid[2], scan_results.aps[i].bssid[3],
			        scan_results.aps[i].bssid[4], scan_results.aps[i].bssid[5],
			        scan_results.aps[i].rssi,
			        scan_results.aps[i].channel,
			        auth_str[scan_results.aps[i].auth_mode]);
		}
	} else {
		LOG_ERR("WiFi scan failed or timed out\n");
	}

	// Test WiFi connection
	LOG_INF("\n--- Testing WiFi Connection ---\n");
	LOG_INF("Connecting to 'Zaviyar-Home-2G' with WPA2...\n");
	if (rp2350_wifi_connect("Zaviyar-Home-2G", "ZaviyarWasim", WIFI_AUTH_WPA2, 45000)) {
		LOG_INF("Successfully connected to WiFi!\n");
	} else {
		LOG_ERR("Failed to connect to WiFi\n");
	}

	LOG_INF("\n=== All tests complete. Halting. ===\n");
	while (1) {
		k_msleep(100000);
	}*/

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

				LOG_INF("readings ready");
				k_msleep(20);
				populate_next_log_cell();
				LOG_INF("update eink");
				k_msleep(20);
				epaper_render_test();
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