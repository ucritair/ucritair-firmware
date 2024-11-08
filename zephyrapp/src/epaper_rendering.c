#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "epaper_driver.h"
#include "epaper_rendering.h"
#include "airquality.h"

bool epaper_flip_y = false;

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(epaper_rendering, LOG_LEVEL_DBG);

void write_px(uint8_t* image, int o_x, int o_y, bool val)
{
	if (epaper_flip_y)
	{
		o_y = EPD_IMAGE_H - o_y;
		o_x = EPD_IMAGE_W - o_x;
	}

	int y = EPD_IMAGE_W - o_x;
	int x = o_y;

	if (x > EPD_IMAGE_H || y > EPD_IMAGE_W) return;

	int pxcount = (y * EPD_IMAGE_H) + x;
	int bytecount = pxcount >> 3;
	int bitcount = pxcount & 0b111;

	if (bytecount >= EPD_IMAGE_BYTES) return;

	if (val)
	{
		image[bytecount] |= 1<<(7-bitcount);
	}
	else
	{
		image[bytecount] &= ~(1<<(7-bitcount));
	}
}

#include "font8x8_basic.h"

void write_char(uint8_t* image, int x, int y, int scale, char c)
{
	for (int bx = 0; bx < 8; bx++)
	{
		for (int by = 0; by < 8; by++)
		{
			if (font8x8_basic[(int)c][by] & (1<<bx))
			{
				for (int sx = 0; sx < scale; sx++)
				{
					for (int sy = 0; sy < scale; sy++)
					{
						write_px(image, x + ((bx)*scale)+sx, y + ((by)*scale)+sy, 1);
					}
				}
			}
		}
	}
}

void write_str(uint8_t* image, int x, int y, int scale, char* str)
{
	int ox = x;
	while (*str)
	{
		write_char(image, x, y, scale, *str);
		x += 8*scale;
		if ((*str) == '\n')
		{
			x = ox;
			y += 8*scale;
		}
		str++;
	}
}

bool sprite_read_px(struct epaper_image_asset* src, int x, int y)
{
	int pxcount = (y * src->stride) + x;
	int bytecount = pxcount >> 3;
	int bitcount = pxcount & 0b111;

	return (src->bytes[bytecount] >> (7-bitcount)) & 1;
}

void blit_image(uint8_t* target, struct epaper_image_asset* src, int x, int y)
{
	LOG_INF("Blit x=%d y=%d w=%d h=%d", x, y, src->w, src->h);
	for (int dx = 0; dx < src->w; dx++)
	{
		for (int dy = 0; dy < src->h; dy++)
		{
			write_px(target, x+dx, y+dy, sprite_read_px(src, dx, dy));
		}
	}
}


uint8_t test_image[EPD_IMAGE_BYTES] = {0};

int step = 0;
int steps[] = {64, 30, 75, 71, 13, 81, 49, 3, 79, 95, 78, 27, 22, 65, 67, 19, 94, 34, 92, 38, 3, 19, 68, 2, 38, 84, 35, 11, 78, 100, 53, 51, 63, 44, 28, 14, 55, 64, 46, 44, 41, 47, 58, 8, 16, 18, 13, 38, 30, 77, 91, 82, 25, 66, 51, 70, 53, 46, 4, 55, 50, 80, 39, 36, 21, 60, 50, 12, 40, 13, 97, 81, 65, 28, 68, 60, 72, 66, 34, 0, 10, 56, 65, 14, 3, 21, 70, 12, 54, 46, 92, 1, 96, 74, 54, 58, 21, 47, 87, 56};

float make_bs_number(float base, float range)
{
	float s = steps[step];
	s /= 100;
	s *= range;
	base += s;
	return base;
}

void epaper_render_test()
{
	memset(test_image, 0, sizeof(test_image));

	char buf[256] = {0};
	// snprintf(buf, 256, 
	// 	"%.0fppm", (double)current_readings.sunrise.ppm_filtered_compensated);

// 	write_str(test_image, 10, 10, 4, buf);

// 	int row = 0;
#define fwrite_str(x, y, s, str, ...) snprintf(buf, sizeof(buf), str, ##__VA_ARGS__); write_str(test_image, x, y, s, buf);

// 	fwrite_str("LPS22HH: Temp %.1fC", (double)current_readings.lps22hh.temp);
// 	fwrite_str("         Pressure %.1fhPa", (double)current_readings.lps22hh.pressure);

// 	fwrite_str("SEN55: Temp: %.1fC", (double)current_readings.sen5x.temp_degC);
// 	fwrite_str("       PM 1.0: %.1f |  2.5: %.1f", (double)current_readings.sen5x.pm1_0, (double)current_readings.sen5x.pm2_5)
// 	fwrite_str("          4.0: %.1f | 10.0: %.1f", (double)current_readings.sen5x.pm4_0, (double)current_readings.sen5x.pm10_0)
// 	fwrite_str("       %.1f%%RH VOC: %.1f", (double)current_readings.sen5x.humidity_rhpct, (double)current_readings.sen5x.voc_index)
// 	fwrite_str("       NOX: %.1f", (double)current_readings.sen5x.nox_index)

// 	fwrite_str("Sunrise: Temp: %.1fC", (double)current_readings.sunrise.temp);
// fwrite_str("    meow   ");

	blit_image(test_image, &epaper_image_unicorn_default, 0, 0);
	blit_image(test_image, &epaper_image_cloud_smoke, 0, epaper_image_unicorn_default.h);

	fwrite_str(128, 20, 2, "%.0f", (double)current_readings.sunrise.ppm_filtered_compensated);
	fwrite_str(EPD_IMAGE_W-(8*3), 20, 1, "ppm\nCO2");
	fwrite_str(128, 40, 2, "%.1f", (double)current_readings.sen5x.pm1_0);
	fwrite_str(EPD_IMAGE_W-(8*5), 40, 1, "ug/m3\nPM2.5");
	fwrite_str(128, 60, 1, "%.0f NOX / %.0f VOC", (double)current_readings.sen5x.nox_index, (double)current_readings.sen5x.voc_index);
	fwrite_str(128, 70, 1, "%.0f C / %.0f%% RH", (double)current_readings.sen5x.temp_degC/1000., (double)current_readings.sen5x.humidity_rhpct);
	// fwrite_str(128, 90, 1, "1 uCov/hr");
	fwrite_str(128, 100, 1, "75%% AQI");
	// fwrite_str(128, 108, 1, "15:35 - ")

	pc_set_mode(false);
	cmd_turn_on_and_write(test_image);
	pc_set_mode(true);
}

