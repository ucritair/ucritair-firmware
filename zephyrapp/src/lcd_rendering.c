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
#include "batt.h"

#include "cat_pet.h"
#include "cat_item.h"

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
bool cat_game_running = false;

void lcd_render_diag()
{
	int last_sensor_update = 0;
	int last_flash_log = 0;
	int last_eink_update = 0;

#ifndef MINIMIZE_GAME_FOOTPRINT
	int slept = get_current_rtc_time() - went_to_sleep_at;
	if (slept < 0) slept = 0;

	LOG_INF("about to CAT_init(slept=%d)", slept);

	CAT_init(slept);
	cat_game_running = true;
#endif

	NRF_SPIM4->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M32;

	int last_frame_time = 1;
	int last_ms = 1;

	int last_lockmask = 0;

	int last_button_pressed = 0;

	while (1)
	{
#ifndef MINIMIZE_GAME_FOOTPRINT
		if (!in_debug_menu)
		{
			CAT_tick_logic();
		}
#endif

		guy_is_wearing_mask = CAT_gear_status(mask_item);
		if (CAT_room_find(purifier_item) != -1 && CAT_room_find(uv_item) != -1)
		{
			guy_happiness = 2;
		}
		else if ((pet.vigour + pet.focus + pet.spirit) < (6 * 3))
		{
			guy_happiness = 0;
		}
		else
		{
			guy_happiness = 1;
		}

		touch_update();
		imu_update();
		update_rtc();
		update_buttons();

		if (adc_get_voltage() < 3.6)
		{
			power_off(0, true);
		}

		if (current_buttons == (CAT_BTN_MASK_SELECT|CAT_BTN_MASK_START|CAT_BTN_MASK_UP))
		{
			in_debug_menu = true;
		}

		int now = k_uptime_get();
		last_frame_time = now - last_ms;
		last_ms = now;

		if (current_buttons || touch_pressure)
		{
			last_button_pressed = now;
			set_backlight(BACKLIGHT_FULL);
		}

		int time_since_buttons = now - last_button_pressed;

		if (time_since_buttons > 45*1000)
		{
			set_backlight(BACKLIGHT_DIM);
		}

		if (time_since_buttons > 120*1000)
		{
			// TODO: Save game
			epaper_render_test();
			power_off(sensor_wakeup_rate*1000, false);
		}

		int lockmask = 0;

		while (!write_done) {
			lockmask |= 1<<(LCD_FRAMEBUFFER_SEGMENTS-1);
			k_usleep(250);
		} // TODO: Move things around to fit game logic before this

		for (int step = 0; step < LCD_FRAMEBUFFER_SEGMENTS; step++)
		{
#ifdef LCD_FRAMEBUFFER_A_B
			int post_buf_num = (step+1)%2;
			int post_offset = LCD_FRAMEBUFFER_H*((step+(LCD_FRAMEBUFFER_SEGMENTS-1))%LCD_FRAMEBUFFER_SEGMENTS);
			lcd_flip(lcd_framebuffer_pair[post_buf_num], post_offset);

			int work_buf_num = step%2;
			lcd_framebuffer = lcd_framebuffer_pair[work_buf_num];
#endif

			framebuffer_offset_h = step*LCD_FRAMEBUFFER_H;

			// LOG_INF("post %d/%d work %d/%d", post_buf_num, post_offset, work_buf_num, framebuffer_offset_h);

			if (in_debug_menu)
			{
				// const uint16_t colors[] = {0, 7 << (5+6), 7 << 5, 7};
				// for (int p = 0; p < LCD_FRAMEBUFFER_PIXELS; p++)
				// {
				// 	lcd_framebuffer[p] = colors[step];
				// }
				memset(lcd_framebuffer, 0, LCD_FRAMEBUFFER_PIXELS*2);
				draw_debug_menu();
			}
			else
			{
#ifndef MINIMIZE_GAME_FOOTPRINT
				CAT_tick_render(step);
#endif
			}

			if (show_fps && step==0)
			{
				char buf[32] = {0};
				snprintf(buf, sizeof(buf)-1, "%02x %2d FPS", last_lockmask, 1000/last_frame_time);
				lcd_write_str(0x0ee0, 240-(strlen("XX XX FPS")*8), 0, buf);
			}

#ifndef LCD_FRAMEBUFFER_A_B
			lcd_flip(lcd_framebuffer, framebuffer_offset_h);
#endif

			if (step != (LCD_FRAMEBUFFER_SEGMENTS-1))
			{
				while (!write_done) {
					lockmask |= (1<<step);
					k_usleep(250);
				}
			}
		}

		last_lockmask = lockmask;


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

		if ((k_uptime_get() - last_flash_log) > (sensor_wakeup_rate*1000) && is_ready_for_aqi_logging())
		{
			populate_next_log_cell();
			last_flash_log = k_uptime_get();
		}
	}
}
