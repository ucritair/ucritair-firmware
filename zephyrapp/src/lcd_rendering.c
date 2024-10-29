#include <string.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lcd_rendering, LOG_LEVEL_DBG);
#include "lcd_driver.h"

#define guint8 uint8_t
#define guint int
#include "img.c"

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

#define get_image_px(x, y) ((uint16_t*)gimp_image.pixel_data)[(y * gimp_image.width) + x]

void lcd_render_diag()
{
	int x = 75;
	int y = 100;
	int dx = 0;
	int step = 3;

	int last_sensor_update = 0;

	while (1)
	{
		// dx += step;

		// if (dx > 100 || dx < 0) step *= -1;

		memset(lcd_framebuffer, 0, sizeof(lcd_framebuffer));

		uint16_t transparent = get_image_px(0, 0);

		for (int px = 0; px < gimp_image.width; px++)
		{
			for (int py = 0; py < gimp_image.height; py++)
			{
				uint16_t pixel = get_image_px(px, py);

				if (pixel != transparent)
				{
					lcd_framebuffer[((y+py+dx)*LCD_IMAGE_W) + (x+px)] = pixel;
				}
			}
		}

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
			current_readings.sunrise.uptime_last_updated, current_readings.sunrise.ppm_filtered_compensated);
		written += snprintf(buf+written, sizeof(buf)-written-1,
			"SEN5x @ %lldms\nPM1.0: %.1f; PM2.5: %.1f\nPM4.0: %.1f; PM10.0: %.1f\nHumidity: %.1f%%RH; Temp: %.1fC\nVOC: %.1f; NOX: %.1f\n\n",
			current_readings.sen5x.uptime_last_updated, (double)current_readings.sen5x.pm1_0, (double)current_readings.sen5x.pm2_5,
			(double)current_readings.sen5x.pm4_0, (double)current_readings.sen5x.pm10_0, (double)current_readings.sen5x.humidity_rhpct,
			(double)current_readings.sen5x.temp_degC, (double)current_readings.sen5x.voc_index, (double)current_readings.sen5x.nox_index);
		written += snprintf(buf+written, sizeof(buf)-written-1, "Meow :3");
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

		lcd_flip();

		if ((k_uptime_get() - last_sensor_update) > 5000)
		{
			sensor_read_once();
			last_sensor_update = k_uptime_get();
		}
	}
}