#include <string.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lcd_rendering, LOG_LEVEL_DBG);
#include "lcd_driver.h"

#include "airquality.h"

#include "buttons.h"
#include "touch.h"

extern char font8x8_basic[128][8];

void lcd_write_char(uint16_t color, int x, int y, char c)
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
						int p = ((x+bx)*scale)+sx + ((((y+by)*scale)+sy)*LCD_IMAGE_W);
						if (p > (sizeof(lcd_framebuffer)/sizeof(lcd_framebuffer[0]))) continue;
						lcd_framebuffer[p] = color;
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

#include "cat_main.h"

bool in_debug_menu = false;
bool show_fps = true;

void lcd_render_diag()
{

	int last_sensor_update = 0;

	LOG_INF("About to CAT_init");

#ifndef MINIMIZE_GAME_FOOTPRINT
	CAT_init();
#endif

	imu_init();

	NRF_SPIM4->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M32;

	int last_ms = 1;

	while (1)
	{
		// dx += step;

		// if (dx > 100 || dx < 0) step *= -1;

		uint32_t start = k_cycle_get_32();
		int start_ms = k_uptime_get();

		memset(lcd_framebuffer, 0, sizeof(lcd_framebuffer));

		uint32_t after_memset = k_cycle_get_32();

		if (in_debug_menu)
		{
			draw_debug_menu();
		}
		else
		{
#ifndef MINIMIZE_GAME_FOOTPRINT
			CAT_tick();
#endif
		}

		uint32_t after_blit = k_cycle_get_32();

		uint8_t buttons = get_buttons();
		// LOG_DBG("Buttons: %02x", buttons);
		touch_update();
		imu_update();

		if (show_fps)
		{
			char buf[32] = {0};
			snprintf(buf, sizeof(buf)-1, "%d FPS", 1000/last_ms);
			lcd_write_str(0x0ee0, 240-(strlen("XX FPS")*8), 0, buf);
		}

		

		uint32_t after_printf = k_cycle_get_32();
		// LOG_DBG("write '%s'", buf);
		// lcd_write_str(0, 0, buf);

		if ((buttons & CAT_BTN_MASK_SELECT) && (buttons & CAT_BTN_MASK_START))
		{
			in_debug_menu = true;
		}
		

		uint32_t after_text = k_cycle_get_32();

		lcd_flip();

		while (!write_done) {k_msleep(1);} // TODO: Move things around to fit game logic before this

		uint32_t after_flip = k_cycle_get_32();

		int end_ms = k_uptime_get();

		last_ms = end_ms-start_ms;

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