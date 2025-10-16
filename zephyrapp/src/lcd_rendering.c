#include <string.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lcd_rendering, LOG_LEVEL_DBG);
#include "lcd_driver.h"

#include "buttons.h"
#include "touch.h"
#include "rtc.h"
#include "batt.h"
#include "flash.h"
#include "sensor_hal.h"
#include "ble.h"
#include "imu.h"
#include "power_control.h"
#include "epaper_rendering.h"
#include "debugmenu.h"

#include "cat_pet.h"
#include "cat_item.h"
#include "cat_item.h"
#include "cat_core.h"
#include "cat_aqi.h"
#include "item_assets.h"
#include "cat_pet.h"
#include "cat_room.h"
#include "menu_system.h"


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
bool show_fps = false;
bool cat_game_running = false;
int last_button_pressed = 0;
uint64_t slept_s = 0;
uint64_t last_dump = 0;

void lcd_keep_awake()
{
	LOG_DBG("lcd_keep_awake()");
	last_button_pressed = k_uptime_get();
}

void lcd_render_diag()
{
	int last_sensor_update = 0;
	int last_flash_log = 0;

#ifndef MINIMIZE_GAME_FOOTPRINT
	slept_s = get_current_rtc_time() - sleep_timestamp;
	if (slept_s < 0) slept_s = 0;

	LOG_INF("about to CAT_init(slept=%d)", slept_s);
	
	CAT_init();

	cat_game_running = true;
#endif

	NRF_SPIM4->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M32;

	int last_frame_time = 1;
	int last_ms = 1;

	int last_lockmask = 0;

	bool charging_last_frame = false;

	while (1)
	{
#ifndef MINIMIZE_GAME_FOOTPRINT
		if (!in_debug_menu)
		{
			CAT_tick_logic();
		}
#endif

		ble_update();

		pet_mask = CAT_inventory_count(mask_item) > 0;
		if (CAT_room_prop_lookup(prop_purifier_item) != -1 && CAT_room_prop_lookup(prop_uv_lamp_item) != -1)
		{
			pet_mood = 2;
		}
		else if ((pet.vigour + pet.focus + pet.spirit) < (6 * 3))
		{
			pet_mood = 0;
		}
		else
		{
			pet_mood = 1;
		}

		memcpy(pet_name, pet.name, sizeof(pet_name));
		pet_level = pet.level;

		touch_update();
		imu_update();
		update_rtc();
		update_buttons();

		bool is_charging = get_is_charging();

		if (current_buttons == (CAT_BTN_MASK_SELECT|CAT_BTN_MASK_START|CAT_BTN_MASK_UP))
		{
			in_debug_menu = true;
		}

		int now_ms = k_uptime_get();
		last_frame_time = now_ms - last_ms;
		last_ms = now_ms;

		if (current_buttons || touch_pressure || co2_calibrating || (charging_last_frame != is_charging))
		{
			last_button_pressed = now_ms;
			set_backlight(screen_brightness);
		}

		int time_since_buttons = now_ms - last_button_pressed;

		if (time_since_buttons > sleep_after_seconds*1000)
		{
			if (!is_charging)
			{
				LOG_INF("Sleeping");
				// TODO: Save game
				epaper_render_test();
				power_off(sensor_wakeup_period*1000, false);
			}
			else
			{
				set_backlight(0);
				// set_backlight(MAX(10, screen_brightness>>2));
			}
		}
		else if (time_since_buttons > dim_after_seconds*1000)
		{
			set_backlight(MAX(10, screen_brightness>>1));
		}

		charging_last_frame = is_charging;

		//////////////////////////////////////////////////////////
		// MISC. UPDATES

		if(CAT_get_battery_pct() <= CAT_CRITICAL_BATTERY_PCT)
			persist_flags |= CAT_PERSIST_CONFIG_FLAG_BATTERY_ALERT;
		else if(CAT_get_battery_pct() >= CAT_SAFE_BATTERY_PCT)
			persist_flags &= ~CAT_PERSIST_CONFIG_FLAG_BATTERY_ALERT;

		int lockmask = 0;

		while (!write_done)
		{
			lockmask |= 1<<(LCD_FRAMEBUFFER_SEGMENTS-1);
			k_usleep(250);
		} // TODO: Move things around to fit game logic before this

		for (int step = 0; step < LCD_FRAMEBUFFER_SEGMENTS; step++)
		{
			framebuffer_offset_h = LCD_FRAMEBUFFER_H * step;

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
				CAT_set_render_cycle(step);
				CAT_tick_render();		
#endif
			}

			if (show_fps && step==0)
			{
				char buf[32] = {0};
				snprintf(buf, sizeof(buf)-1, "%02x %2d FPS", last_lockmask, 1000/last_frame_time);
				lcd_write_str(0x0ee0, 240-(strlen("XX XX FPS")*8), 0, buf);
			}

#ifdef LCD_FRAMEBUFFER_A_B
			int post_buf_num = step % 2;
			int work_buf_num = (step+1) % 2;
			int post_offset = LCD_FRAMEBUFFER_H * step;

			if(CAT_get_screen_orientation() == CAT_SCREEN_ORIENTATION_DOWN)
			{
				post_buf_num = (step+1) % 2;
				work_buf_num = step % 2;
				post_offset = LCD_FRAMEBUFFER_H * ((step + LCD_FRAMEBUFFER_SEGMENTS - 1) % LCD_FRAMEBUFFER_SEGMENTS);

				for(int y = 0; y < LCD_FRAMEBUFFER_H/2; y++)
				{
					for(int x = 0; x < LCD_FRAMEBUFFER_W; x++)
					{
						int y_flip = LCD_FRAMEBUFFER_H - y - 1;
						int x_flip = LCD_FRAMEBUFFER_W - x - 1;
						uint16_t temp = lcd_framebuffer[y_flip * LCD_FRAMEBUFFER_W + x_flip];
						lcd_framebuffer[y_flip * LCD_FRAMEBUFFER_W + x_flip] = lcd_framebuffer[y * LCD_FRAMEBUFFER_W + x];
						lcd_framebuffer[y * LCD_FRAMEBUFFER_W + x] = temp;
					}
				}
			}

			lcd_flip(lcd_framebuffer_pair[post_buf_num], post_offset);
			lcd_framebuffer = lcd_framebuffer_pair[work_buf_num];
#else
			lcd_flip(lcd_framebuffer, framebuffer_offset_h);
#endif

			if (step != (LCD_FRAMEBUFFER_SEGMENTS-1))
			{
				while (!write_done)
				{
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

		if(CAT_poll_eink_update_flag())
			CAT_eink_execute_update();

		int battery = CAT_get_battery_pct();
		if (battery == 0)
       		CAT_shutdown();
		else if(battery <= 10)
			epaper_render_protected_off();

		if ((k_uptime_get() - last_flash_log) > (sensor_wakeup_period*1000) && is_ready_for_aqi_logging())
		{
			populate_next_log_cell();
			last_flash_log = k_uptime_get();
		}
	}
}
