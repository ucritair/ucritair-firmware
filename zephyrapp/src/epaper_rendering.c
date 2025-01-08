#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "epaper_driver.h"
#include "epaper_rendering.h"
#include "airquality.h"
#include "rtc.h"
#include "imu.h"
#include "misc.h"

#include "cat_item.h"
#include "cat_version.h"
#include "cat_core.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(epaper_rendering, LOG_LEVEL_DBG);

void write_px(uint8_t* image, int o_x, int o_y, bool val)
{
	if (imu_recognized_upside_down)
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


PERSIST_RAM uint8_t epaper_framebuffer[EPD_IMAGE_BYTES];
PERSIST_RAM uint8_t old_epaper_framebuffer[EPD_IMAGE_BYTES];
PERSIST_RAM uint8_t framebuffer_fast_update_count;

void epaper_render_test()
{
	memset(epaper_framebuffer, 0, sizeof(epaper_framebuffer));

	char buf[256] = {0};

#define fwrite_str(x, y, s, str, ...) snprintf(buf, sizeof(buf), str, ##__VA_ARGS__); write_str(epaper_framebuffer, x, y, s, buf);

	struct tm t;
	time_t now = get_current_rtc_time();
	gmtime_r(&now, &t);

	struct epaper_image_asset* selected_unicorn = &epaper_image_unicorn_default;
	struct epaper_image_asset* selected_cloud = &epaper_image_cloud_default;

	int temp_idx, co2_idx, pm_idx, voc_idx, nox_idx;
	CAT_AQI_quantize(&temp_idx, &co2_idx, &pm_idx, &voc_idx, &nox_idx);

	if (guy_is_wearing_mask)
	{
		selected_unicorn = &epaper_image_unicorn_mask;
	}
	else
	{
		if (guy_happiness == 2)
		{
			selected_unicorn = &epaper_image_unicorn_happy;
		}
		else if (guy_happiness == 0)
		{
			selected_unicorn = &epaper_image_unicorn_sad;
		}
	}

	float score = CAT_AQI_aggregate();

	if (score < 25)
	{
		selected_cloud = &epaper_image_cloud_smoke;
	}
	else if (score < 50)
	{
		selected_cloud = &epaper_image_cloud_sad;
	}
	else if (score > 75)
	{
		selected_cloud = &epaper_image_cloud_happy;
	}

	blit_image(epaper_framebuffer, selected_unicorn, 0, 0);
	blit_image(epaper_framebuffer, selected_cloud, 0, selected_unicorn->h);

	fwrite_str(128, 20, 2, "%.0f", (double)current_readings.sunrise.ppm_filtered_compensated);
	fwrite_str(EPD_IMAGE_W-(8*3), 20, 1, "ppm\nCO2");
	fwrite_str(128, 40, 2, "%.1f", (double)current_readings.sen5x.pm2_5);
	fwrite_str(EPD_IMAGE_W-(8*5), 40, 1, "ug/m3\nPM2.5");

	if (current_readings.sen5x.nox_index && current_readings.sen5x.voc_index)
	{
		fwrite_str(128, 60, 1, "%.0f NOX / %.0f VOC", (double)current_readings.sen5x.nox_index, (double)current_readings.sen5x.voc_index);
	}
	
	fwrite_str(128, 70, 1, "%.0f C / %.0f%% RH", (double)current_readings.sen5x.temp_degC, (double)current_readings.sen5x.humidity_rhpct);
	fwrite_str(128, 80, 1, "%.1f%% rebreathed", ((((double)current_readings.sunrise.ppm_filtered_compensated)-420.)/38000.)*100.);
	fwrite_str(128, 90, 1, "uCritAQI %.1f%%", score);
	fwrite_str(128, 100, 1, "at %2d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);

	fwrite_str(0, EPD_IMAGE_H-8, 1, "\"%s\"", guy_name);

	imu_update();

	pc_set_mode(false);
	if (is_first_init || framebuffer_fast_update_count>50)
	{
		LOG_DBG("Opted to global update, is_first_init=%d, framebuffer_fast_update_count=%d", is_first_init, framebuffer_fast_update_count);
		is_first_init = false;
		cmd_turn_on_and_write(epaper_framebuffer);
		framebuffer_fast_update_count = 0;
	}
	else
	{
		LOG_DBG("Opted to fast update, framebuffer_fast_update_count=%d", framebuffer_fast_update_count);
		cmd_turn_on_and_write_fast(old_epaper_framebuffer, epaper_framebuffer);
		framebuffer_fast_update_count++;
	}
	pc_set_mode(true);

	memcpy(old_epaper_framebuffer, epaper_framebuffer, sizeof(epaper_framebuffer));
}

void epaper_render_protected_off()
{
	memset(epaper_framebuffer, 0, sizeof(epaper_framebuffer));

#define S(x) #x

	char buf[256] = {0};

	write_str(epaper_framebuffer, 10, 10, 2, "Device is");
	write_str(epaper_framebuffer, 10, 26, 2, "protected-off");
	write_str(epaper_framebuffer, 10, 56, 1, "Press RESET to power on");
	write_str(epaper_framebuffer, 10, 76, 1,  "SYS  v." SYS_FW_VERSION);
	fwrite_str(10, 84, 1, "GAME v.%d.%d.%d.%d", CAT_VERSION_MAJOR, CAT_VERSION_MINOR, CAT_VERSION_PATCH, CAT_VERSION_PUSH);

	pc_set_mode(false);
	cmd_turn_on_and_write(epaper_framebuffer);
	pc_set_mode(true);
}