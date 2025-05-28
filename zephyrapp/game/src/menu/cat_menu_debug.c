#include "cat_menu.h"

#include "cat_input.h"
#include "cat_version.h"
#include "cat_gui.h"
#include "cat_main.h"
#include "cat_pet.h"
#include "cat_room.h"
#include "config.h"
#include <stdint.h>
#include "cat_aqi.h"
#include "sprite_assets.h"

static enum
{
	SYSTEM,
	TIME,
	DECO,
	INPUT,
	AQI,
	LAST
} page = SYSTEM;

void CAT_MS_debug(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_debug);
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			if(CAT_input_pulse(CAT_BUTTON_LEFT))
			{
				if(page == 0)
					page = LAST-1;
				else
					page -= 1;
			}
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
			{
				page += 1;
				if(page >= LAST)
					page = 0;
			}	
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_debug()
{
	switch(page)
	{
		case SYSTEM:
			CAT_gui_title(true, NULL, &icon_exit_sprite, "SYSTEM");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			CAT_gui_textf
			(
				"Game v%d.%d.%d.%d\nSave v%d.%d.%d.%d\n",
				CAT_VERSION_MAJOR, CAT_VERSION_MINOR,
				CAT_VERSION_PATCH, CAT_VERSION_PUSH,
				saved_version_major, saved_version_minor,
				saved_version_patch, saved_version_push
			);
			
#define TOSTRING_INNER(x) #x
#define TOSTRING(x) TOSTRING_INNER(x)
#if defined(CAT_DESKTOP)
			CAT_gui_text("DESKTOP\n");
#elif defined(CAT_EMBEDDED)
			CAT_gui_text("EMBEDDED\n");
#endif

			if(CAT_check_save_flag(CAT_SAVE_FLAG_DEVELOPER_MODE))
				CAT_gui_text("DEVELOPER MODE\n");
		break;
		case TIME:
			CAT_gui_title(true, NULL, &icon_exit_sprite, "TIME");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			CAT_gui_textf("Slept: %ds\n", CAT_get_slept_s());
			CAT_gui_textf("Life: %0.0fs/%0.0fs\n", CAT_timer_get(pet.life_timer_id), timetable.duration[pet.life_timer_id]);
			CAT_gui_textf("Stat: %0.0fs/%0.0fs\n", CAT_timer_get(pet.stat_timer_id), timetable.duration[pet.stat_timer_id]);
			CAT_gui_textf("Earn: %0.0fs/%0.0fs\n", CAT_timer_get(room.earn_timer_id), timetable.duration[room.earn_timer_id]);
			CAT_gui_textf("Pet: %0.0fs/%0.0fs\n", CAT_timer_get(pet.petting_timer_id), timetable.duration[pet.petting_timer_id]);
			CAT_gui_textf("Pets: %d/5\n", pet.times_pet);
			CAT_gui_textf("Milks: %d/3\n", pet.times_milked);
			CAT_gui_textf("E-Ink: %0.0fs/%0.0fs\n", time_since_eink_update, eink_update_time_threshold);
			int active_timers = 0;
			for(int i = 0; i < CAT_TIMETABLE_MAX_LENGTH; i++)
			{
				if(timetable.active[i])
					active_timers += 1;
			}
			CAT_gui_textf("Timers: %d\n", active_timers);
		break;
		case DECO:
			CAT_gui_title(true, NULL, &icon_exit_sprite, "DECO");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			for(int y = 0; y < CAT_GRID_HEIGHT; y++)
			{
				for(int x = 0; x < CAT_GRID_WIDTH; x++)
				{
					if(CAT_is_grid_point_free((CAT_ivec2){x, y}))
						CAT_strokeberry(gui.cursor.x, gui.cursor.y, 8, 8, CAT_BLACK);
					else
						CAT_fillberry(gui.cursor.x, gui.cursor.y, 8, 8, CAT_BLACK);
					gui.cursor.x += 8;
				}
				gui.cursor.x = gui.margin;
				gui.cursor.y += 8;
			}
		break;
		case INPUT:
		{
			CAT_gui_title(true, NULL, &icon_exit_sprite, "INPUT");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

			CAT_gui_text("Mask: ");
			uint16_t input_mask = CAT_get_buttons();
			for(int i = 0; i < CAT_BUTTON_LAST; i++)
			{
				bool state = (input_mask & (1 << i)) > 0;
				CAT_gui_textf("%d", state);
			}
			CAT_gui_line_break();

			CAT_gui_textf("Downtime: %0.1f\n", CAT_input_time_since_last());
			
		}
		break;
		case AQI:
		{
			CAT_gui_title(true, NULL, &icon_exit_sprite, "AQI");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			CAT_gui_textf("RH: %f\n", readings.sen5x.humidity_rhpct);
			CAT_gui_textf("CO2: %f\n", readings.sunrise.ppm_filtered_compensated);
			CAT_gui_textf("PM: %f\n", readings.sen5x.pm2_5);
			CAT_gui_textf("NOX: %f\n", readings.sen5x.nox_index);
			CAT_gui_textf("VOC: %f\n", readings.sen5x.voc_index);
			CAT_gui_textf("TMP: %f%s\n", CAT_AQ_map_celsius(readings.lps22hh.temp), CAT_AQ_get_temperature_unit_string());
			CAT_gui_textf("AQI: %f\n", CAT_AQI_aggregate());
		}
		break;
		default:
		{
			CAT_gui_title
			(
				true,
				NULL, &icon_exit_sprite,
				"LAST"
			);
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			CAT_gui_set_flag(CAT_GUI_TEXT_WRAP);
			CAT_gui_text("You shouldn't be here");
			break;
		}
	}	
}