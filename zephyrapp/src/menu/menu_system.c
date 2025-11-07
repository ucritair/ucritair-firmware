#include "menu_system.h"

#include "cat_room.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"
#include "cat_render.h"
#include "cat_version.h"
#include "cat_menu.h"
#include "cat_item.h"
#include "cat_item.h"

#include "misc.h"
#include "menu_time.h"
#include "power_control.h"
#include "rtc.h"
#include "epaper_rendering.h"
#include "sdcard.h"
#include "flash.h"
#include "lcd_driver.h"
#include "lcd_rendering.h"
#include "batt.h"
#include "sprite_assets.h"
#include "item_assets.h"

#include <zephyr/kernel.h>

#include <stddef.h>

int system_menu_selector = 0;

char* system_menu_note = "";

bool co2_calibrating = false;
bool did_co2_cal = false;

#define CO2_CAL_TIME (5*60)
// set to 5 min
int co2_calibration_start_time = 0;

typedef void (*menu_t)();

void menu_t_back()
{
	CAT_pushdown_pop();
}

void menu_t_go_time()
{
	CAT_pushdown_push(CAT_MS_time);
}

void menu_t_go_co2()
{
	co2_calibrating = true;
	did_co2_cal = false;
	co2_calibration_start_time = k_uptime_get_32();
}

void menu_t_sleep()
{
	power_off(sensor_wakeup_period*1000, false);
}

void menu_t_power_off()
{
	epaper_render_protected_off();
	power_off(0, true);
}

void menu_t_erase_logs()
{
	flash_erase_all_cells();
	system_menu_note = "Done :)";
}

void menu_t_write_logs()
{
	enum sdcard_result res = write_log_to_sdcard();

	if (res == OK)
		system_menu_note = "Written :)";
	else if (res == FAIL_INIT)
		system_menu_note = "Failed to init card";
 	else if (res == FAIL_MOUNT)
 		system_menu_note = "Failed to mount FS";
 	else if (res == FAIL_WRITE)
 		system_menu_note = "Failed to write data";
 	else if (res == FAIL_MKDIR)
 		system_menu_note = "Failed to mkdir";
 	else if (res == FAIL_CREATE)
 		system_menu_note = "Failed to creat";
 	else if (res == FAIL_CLOSE)
 		system_menu_note = "Failed closing file";
 	else
 		system_menu_note = "Unknown error saving";
}

void menu_t_update_eink()
{
	epaper_render_test();
	system_menu_note = "Done :)";
}

void menu_t_reset()
{
	cat_game_running = 0;
	flash_nuke_tomas_save();
	power_off(0, false);
}

void menu_t_bright_down()
{
	screen_brightness = MAX(screen_brightness-5, 10);
}

void menu_t_bright_up()
{
	screen_brightness = MIN(screen_brightness+5, BACKLIGHT_FULL);
}

struct entry
{
	const char* title;
	menu_t action;
} system_entries[] =
{
	{"SET CLOCK + LOG RATE", menu_t_go_time},
	{"ERASE ON-DEVICE LOGS", menu_t_erase_logs},
	{"WRITE LOGS TO SDCARD", menu_t_write_logs},
	{"CALIBRATE CO2 SENSOR", menu_t_go_co2},
//	{"BRIGHTNESS DOWN", menu_t_bright_down},
//	{"BRIGHTNESS UP", menu_t_bright_up},
//	{"UPDATE EINK", menu_t_update_eink},
//	{"RESET GAME", menu_t_reset},
//	{"POWER OFF", menu_t_power_off},
//	{"SLEEP", menu_t_sleep},
	{"BACK", menu_t_back}
};
#define NUM_MENU_ITEMS (sizeof(system_entries)/sizeof(system_entries[0]))

void CAT_MS_system_menu(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_system_menu);
			system_menu_note = "Some items take time";
			break;
		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_pulse(CAT_BUTTON_UP))
				system_menu_selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				system_menu_selector += 1;
			system_menu_selector = CAT_clamp(system_menu_selector, 0, NUM_MENU_ITEMS-1);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				system_entries[system_menu_selector].action();
			}

			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				if (!co2_calibrating)
				{
					CAT_pushdown_pop();
				}
				else
				{
					co2_calibrating = false;
				}
			}
			break;
		}
		case CAT_FSM_SIGNAL_EXIT:
			if (co2_calibrating)
			{
				co2_calibrating = false;
			}
			break;
	}
}

#define PAD 8
static int cursor_x = PAD;
static int cursor_y = PAD;

static void draw_page(const char* title)
{
	cursor_x = PAD;
	cursor_y = PAD;

	CAT_frameberry(CAT_WHITE);
	cursor_y = CAT_draw_textf(cursor_x, cursor_y, "%s\n", title);
	CAT_rowberry(cursor_y+PAD/2, cursor_y+PAD/2+1, CAT_BLACK);
	cursor_y += PAD;
}

void CAT_render_system_menu()
{
	draw_page(co2_calibrating ? "CO2 CALIBRATION":"SYSTEM MENU");

	if (!co2_calibrating)
	{
		for (int i = 0; i < NUM_MENU_ITEMS; i++)
		{
			cursor_y = CAT_draw_textf
			(
				cursor_x, cursor_y,
				"\1 %s %s\n",
				system_entries[i].title, i == system_menu_selector ? "<" : ""
			);
		}
		cursor_y += CAT_TEXT_LINE_HEIGHT;

		cursor_y = CAT_draw_text(cursor_x, cursor_y, system_menu_note);
		cursor_y += CAT_TEXT_LINE_HEIGHT;

		cursor_y = CAT_draw_text(cursor_x, cursor_y, "SYS v." SYS_FW_VERSION "\n");

		cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Battery: %3d%%\n", get_battery_pct());
	}
	else
	{
		int remaining = CO2_CAL_TIME - ((k_uptime_get_32() - co2_calibration_start_time) / 1000);

		CAT_set_text_mask(PAD, -1, CAT_LCD_SCREEN_W-PAD, -1);
		CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
		cursor_y = CAT_draw_text
		(
			cursor_x, cursor_y,
			"Please go outside and wait while CO2 sensor reading settles before calibration\n"
		);

		if (remaining > 0)
		{
			CAT_draw_sprite_raw(&icon_nosmoke_sprite, 0, cursor_x, cursor_y);
			cursor_y += icon_nosmoke_sprite.height + PAD;
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "%2d:%02d remaining...\n", remaining/60, remaining%60);
		}
		else if (remaining > -1)
		{
			CAT_draw_sprite_raw(&icon_nosmoke_sprite, 0, cursor_x, cursor_y);
			cursor_y += icon_nosmoke_sprite.height + PAD;
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Calibrating...\n");
		}
		else if (remaining > -2)
		{
			CAT_draw_sprite_raw(&icon_nosmoke_sprite, 0, cursor_x, cursor_y);
			cursor_y += icon_nosmoke_sprite.height + PAD;
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Calibrating...\n");		
			if (!did_co2_cal)
			{
				did_co2_cal = true;
				force_abc_sunrise();
				CAT_inventory_add(food_cigs_item, 1);
			}
		}
		else
		{
			CAT_set_text_mask(PAD, -1, CAT_LCD_SCREEN_W-PAD, -1);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
			cursor_y = CAT_draw_text
			(
				cursor_x, cursor_y,
				"Done. Thanks for waiting... Have some cigarettes as a reward:\n"
			);
			CAT_draw_sprite_raw(&cigarette_sprite, 0, cursor_x, cursor_y);
			CAT_pushdown_pop(); //Go back so we dont recal by mistake
		}
	}
}