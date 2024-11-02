#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "epaper_driver.h"
#include "airquality.h"


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(epaper_rendering, LOG_LEVEL_DBG);

void write_px(uint8_t* image, int o_x, int o_y, bool val)
{

	int y = EPD_IMAGE_H - o_x;
	int x = o_y;

	if (x > EPD_IMAGE_W || y > EPD_IMAGE_H) return;

	int pxcount = (y * EPD_IMAGE_W) + x;
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
	snprintf(buf, 256, 
		"%.0fppm", (double)current_readings.sunrise.ppm_filtered_compensated);

	write_str(test_image, 10, 10, 4, buf);

	int row = 0;
#define fwrite_str(str, ...) snprintf(buf, sizeof(buf), str, __VA_ARGS__); write_str(test_image, 10, 45 + ((row++)*8), 1, buf);

	fwrite_str("Temp %.1fC, Pressure %.1fhPa", (double)current_readings.lps22hh.temp, (double)current_readings.lps22hh.pressure);
	fwrite_str("PM 1.0: %.1f/ 2.5: %.1f", (double)current_readings.sen5x.pm1_0, (double)current_readings.sen5x.pm2_5)
	fwrite_str("   4.0: %.1f/10.0: %.1f", (double)current_readings.sen5x.pm4_0, (double)current_readings.sen5x.pm10_0)
	fwrite_str("%.1f%%RH VOC: %.1f NOX: %.1f", (double)current_readings.sen5x.humidity_rhpct, (double)current_readings.sen5x.voc_index, (double)current_readings.sen5x.nox_index)
	fwrite_str("    meow    %s", "");

	pc_set_mode(false);
	cmd_turn_on_and_write(test_image);
	pc_set_mode(true);
}

