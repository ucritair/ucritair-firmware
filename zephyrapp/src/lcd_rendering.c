#include <string.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lcd_rendering, LOG_LEVEL_DBG);
#include "lcd_driver.h"

#include "airquality.h"

#include "buttons.h"

extern char font8x8_basic[128][8];

void lcd_write_char(int x, int y, char c)
{
	for (int bx = 0; bx < 8; bx++)
	{
		for (int by = 0; by < 8; by++)
		{
			if (font8x8_basic[(int)c][by] & (1<<bx))
			{
				const int scale = 1;

				for (int sx = 0; sx < scale; sx++)
				{
					for (int sy = 0; sy < scale; sy++)
					{
						lcd_framebuffer[((x+bx)*scale)+sx + ((((y+by)*scale)+sy)*LCD_IMAGE_W)] = 0xffff;
					}
				}
			}
		}
	}
}

void lcd_write_str(int x, int y, char* str)
{
	int ox = x;
	while (*str)
	{
		lcd_write_char(x, y, *str);
		x += 8;
		if ((*str) == '\n')
		{
			x = ox;
			y += 8;
		}
		str++;
	}
}

extern const uint16_t image_data[];
extern const int image_w, image_h;

#define get_image_px(x, y) image_data[(y * image_w) + x]

extern struct k_poll_signal mipi_dbi_spi_write_done;

uint32_t hack_cyc_before_data_write, hack_cyc_after_data_write, hack_before_blit, hack_after_blit;

void lcd_render_diag()
{
	int x = 75;
	int y = 100;
	int dx = 0;
	int step = 3;

	int last_sensor_update = 0;

	NRF_SPIM4->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M32;

	while (1)
	{
		// dx += step;

		// if (dx > 100 || dx < 0) step *= -1;

		uint32_t start = k_cycle_get_32();
		int start_ms = k_uptime_get();

		memset(lcd_framebuffer, 0, sizeof(lcd_framebuffer));

		uint32_t after_memset = k_cycle_get_32();

		const uint16_t* readptr = &image_data[0];
		uint16_t* writeptr = &lcd_framebuffer[(y*LCD_IMAGE_W) + x];

		for (int row = 0; row < image_h; row++)
		{
			for (int col = 0; col < image_w; col++)
			{
				*(writeptr++) = *(readptr++);
			}

			writeptr += LCD_IMAGE_W-image_w;
		}

		uint32_t after_blit = k_cycle_get_32();

		uint8_t buttons = get_buttons();
		// LOG_DBG("Buttons: %02x", buttons);

		char buf[512] = {0};
		int written = 0;
		written += snprintf(buf+written, sizeof(buf)-written-1, 
			"CAT Test\nButtons: %02x\nUptime: %lldms\n\n",
			buttons, k_uptime_get());
		written += snprintf(buf+written, sizeof(buf)-written-1,
			"LPS22HH @ %lldms\nTemp: %.1fC\nPressure: %.1f?\n\n",
			current_readings.lps22hh.uptime_last_updated, (double)current_readings.lps22hh.temp, (double)current_readings.lps22hh.pressure);
		written += snprintf(buf+written, sizeof(buf)-written-1,
			"Sunrise @ %lldms\nCO2: %.0fppm\n\n",
			current_readings.sunrise.uptime_last_updated, (double)current_readings.sunrise.ppm_filtered_compensated);
		written += snprintf(buf+written, sizeof(buf)-written-1,
			"SEN5x @ %lldms\nPM1.0: %.1f; PM2.5: %.1f\nPM4.0: %.1f; PM10.0: %.1f\nHumidity: %.1f%%RH; Temp: %.1fC\nVOC: %.1f; NOX: %.1f\n\n",
			current_readings.sen5x.uptime_last_updated, (double)current_readings.sen5x.pm1_0, (double)current_readings.sen5x.pm2_5,
			(double)current_readings.sen5x.pm4_0, (double)current_readings.sen5x.pm10_0, (double)current_readings.sen5x.humidity_rhpct,
			(double)current_readings.sen5x.temp_degC, (double)current_readings.sen5x.voc_index, (double)current_readings.sen5x.nox_index);
		written += snprintf(buf+written, sizeof(buf)-written-1, "Meow :3");

		uint32_t after_printf = k_cycle_get_32();
		// LOG_DBG("write '%s'", buf);
		lcd_write_str(0, 0, buf);

		

		if (buttons & CAT_BTN_MASK_UP) y -= step;
		if (buttons & CAT_BTN_MASK_DOWN) y += step;
		if (buttons & CAT_BTN_MASK_LEFT) x -= step;
		if (buttons & CAT_BTN_MASK_RIGHT) x += step;
		if (buttons & CAT_BTN_MASK_START)
		{
			epaper_render_test();
			pc_set_mode(true);
		}
		if (buttons & CAT_BTN_MASK_SELECT)
		{
			test_speaker();
		}

		// struct k_poll_event events[1] = {
	    //     K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL,
	    //                              K_POLL_MODE_NOTIFY_ONLY,
	    //                              &mipi_dbi_spi_write_done),
	    // };

		// k_poll(events, 1, K_FOREVER);
		// k_poll_signal_reset(&mipi_dbi_spi_write_done);

		uint32_t after_text = k_cycle_get_32();

		lcd_flip();

		uint32_t after_flip = k_cycle_get_32();

		int end_ms = k_uptime_get();

		if ((k_uptime_get() - last_sensor_update) > 5000)
		{
			sensor_read_once();
			last_sensor_update = k_uptime_get();


			int cyc_memset = after_memset - start;
			int cyc_blit = after_blit - after_memset;
			int cyc_printf = after_printf - after_blit;
			int cyc_text = after_text - after_printf;
			int cyc_flip = after_flip - after_text;
			LOG_DBG("cyc_memset=%d, cyc_blit=%d, cyc_printf=%d, cyc_text=%d, cyc_flip=%d (blit=%d, spi=%d), total=%dms",
				cyc_memset, cyc_blit, cyc_printf, cyc_text, cyc_flip,
				hack_after_blit-hack_before_blit, hack_cyc_after_data_write-hack_cyc_before_data_write, end_ms-start_ms);
		}
	}
}