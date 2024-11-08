#include <string.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lcd_rendering, LOG_LEVEL_DBG);
#include "lcd_driver.h"

#include "airquality.h"

#include "buttons.h"
#include "touch.h"
#include "rtc.h"

extern char font8x8_basic[128][8];

void lcd_write_char(uint16_t color, int x, int y, char c)
{
	y -= framebuffer_offset_h;

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
						int fy = (((y+by)*scale)+sy);
						int fx = ((x+bx)*scale)+sx;
						
						if (fx < 0 || fx >= LCD_IMAGE_W) continue;
						if (fy < 0 || fy >= LCD_FRAMEBUFFER_H) continue;

						lcd_framebuffer[(fy * LCD_IMAGE_W) + fx] = color;
					}
				}
			}
		}
	}
}

void lcd_write_str(uint16_t color, int x, int y, char* str)
{
	int ox = x;
	while (*str)
	{
		lcd_write_char(color, x, y, *str);
		x += 8;
		if ((*str) == '\n')
		{
			x = ox;
			y += 8;
		}
		str++;
	}
}


uint32_t hack_cyc_before_data_write, hack_cyc_after_data_write, hack_before_blit, hack_after_blit;

extern volatile bool write_done;

int epaper_update_rate = -1;

#include "cat_main.h"

bool in_debug_menu = false;
bool show_fps = true;

void lcd_render_diag()
{

	int last_sensor_update = 0;
	int last_eink_update = 0;

	LOG_INF("About to CAT_init");

#ifndef MINIMIZE_GAME_FOOTPRINT
	CAT_init();
#endif

	NRF_SPIM4->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M32;

	int last_frame_time = 1;
	int last_ms = 1;

	while (1)
	{
#ifndef MINIMIZE_GAME_FOOTPRINT
		CAT_tick_logic();
#endif

		touch_update();
		imu_update();
		update_rtc();

		uint8_t buttons = get_buttons();

		if ((buttons & CAT_BTN_MASK_SELECT) && (buttons & CAT_BTN_MASK_START))
		{
			in_debug_menu = true;
		}

		while (!write_done) {k_msleep(1);} // TODO: Move things around to fit game logic before this

		int now = k_uptime_get();
		last_frame_time = now - last_ms;
		last_ms = now;

		for (int step = 0; step < LCD_FRAMEBUFFER_SEGMENTS; step++)
		{
			framebuffer_offset_h = step*LCD_FRAMEBUFFER_H;

			if (in_debug_menu)
			{
				// const uint16_t colors[] = {0, 7 << (5+6), 7 << 5, 7};
				// for (int p = 0; p < LCD_FRAMEBUFFER_PIXELS; p++)
				// {
				// 	lcd_framebuffer[p] = colors[step];
				// }
				memset(lcd_framebuffer, 0, sizeof(lcd_framebuffer));
				draw_debug_menu();
			}
			else
			{
#ifndef MINIMIZE_GAME_FOOTPRINT
				CAT_tick_render(step);
#endif
			}

			if (show_fps)
			{
				char buf[32] = {0};
				snprintf(buf, sizeof(buf)-1, "%d FPS", 1000/last_frame_time);
				lcd_write_str(0x0ee0, 240-(strlen("XX FPS")*8), 0, buf);
			}

			lcd_flip();

			if (step != (LCD_FRAMEBUFFER_SEGMENTS-1))
			{
				while (!write_done) {k_msleep(1);}
			}
		}

		if ((k_uptime_get() - last_sensor_update) > 5000)
		{
			sensor_read_once();
			last_sensor_update = k_uptime_get();

			// LOG_DBG("cyc_memset=%d, cyc_blit=%d, cyc_printf=%d, cyc_text=%d, cyc_flip=%d (blit=%d, spi=%d), total=%dms",
			// 	cyc_memset, cyc_blit, cyc_printf, cyc_text, cyc_flip,
			// 	hack_after_blit-hack_before_blit, hack_cyc_after_data_write-hack_cyc_before_data_write, end_ms-start_ms);
		}

		if ((k_uptime_get() - last_eink_update) > epaper_update_rate && epaper_update_rate != -1)
		{
			epaper_render_test();
			last_eink_update = k_uptime_get();
		}
	}
}
