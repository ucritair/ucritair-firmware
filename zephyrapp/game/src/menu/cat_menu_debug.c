#include "cat_menu.h"

#include "cat_input.h"
#include "cat_version.h"
#include "cat_gui.h"
#include "cat_main.h"
#include "cat_pet.h"
#include "cat_room.h"
#include <stdint.h>
#include "cat_aqi.h"
#include "sprite_assets.h"
#include "cat_crisis.h"

static enum
{
	SYSTEM,
	TIME,
	DECO,
	INPUT,
	AQI,
	PERSIST,
	LAST
};
int page = SYSTEM;

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
				page -= 1;
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
				page += 1;
			page = wrap(page, LAST);

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
			CAT_gui_title(true, "SYSTEM");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			
			CAT_gui_textf
			(
				"Game v%d.%d.%d.%d\n",
				CAT_VERSION_MAJOR, CAT_VERSION_MINOR,
				CAT_VERSION_PATCH, CAT_VERSION_PUSH
			);
			
#define TOSTRING_INNER(x) #x
#define TOSTRING(x) TOSTRING_INNER(x)
#if defined(CAT_DESKTOP)
			CAT_gui_text("DESKTOP\n");
#elif defined(CAT_EMBEDDED)
			CAT_gui_text("EMBEDDED\n");
#endif

			if(CAT_check_config_flags(CAT_CONFIG_FLAG_DEVELOPER))
				CAT_gui_text("DEVELOPER MODE\n");
			if(CAT_check_config_flags(CAT_CONFIG_FLAG_MIGRATED))
				CAT_gui_text("MIGRATED SAVE\n");
			if(CAT_was_persist_wiped())
				CAT_gui_text("PERSIST RESET\n");
		break;
		
		case TIME:
			CAT_gui_title(true, "TIME");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

			uint64_t now = CAT_get_RTC_now();
			CAT_datetime datetime;
			CAT_make_datetime(now, &datetime);
			CAT_gui_textf("%.2d/%.2d/%.4d %.2d:%.2d:%.2d\n", datetime.month, datetime.day, datetime.year, datetime.hour, datetime.minute, datetime.second);

			CAT_pet_timing_state pet_timing;
			CAT_pet_export_timing_state(&pet_timing);
			
			CAT_make_datetime(pet_timing.last_life_time, &datetime);
			CAT_gui_textf("Life: %d/%d/%d %d:%d:%d\n", datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second);
			CAT_make_datetime(pet_timing.last_stat_time, &datetime);
			CAT_gui_textf("Stat: %d/%d/%d %d:%d:%d\n", datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second);

			CAT_gui_textf("Since Life: %llus/%ds\n", now-pet_timing.last_life_time, CAT_LIFE_TICK_PERIOD);
			CAT_gui_textf("Since Stat: %llus/%ds\n", now-pet_timing.last_stat_time, CAT_STAT_TICK_PERIOD);

			CAT_gui_textf("Slept: %llus\n", CAT_get_slept_s());
			CAT_gui_textf("E-Ink: %llus/%ds\n", now-last_eink_time, EINK_UPDATE_PERIOD);
		break;

		case DECO:
			CAT_gui_title(true, "DECO");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			for(int y = 0; y < CAT_ROOM_GRID_H; y++)
			{
				for(int x = 0; x < CAT_ROOM_GRID_W; x++)
				{
					if(CAT_room_is_cell_free(x, y))
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
			CAT_gui_title(true, "INPUT");
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
			CAT_gui_title(true, "AQI");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

			if(CAT_AQ_logs_initialized())
				CAT_gui_textf("Logs initialized (%d)\n", CAT_get_log_cell_count());
			if(CAT_AQ_sensors_initialized())
				CAT_gui_text("Sensors initialized\n");

			CAT_gui_textf("RH: %f\n", readings.sen5x.humidity_rhpct);
			CAT_gui_textf("CO2: %f\n", readings.sunrise.ppm_filtered_compensated);
			CAT_gui_textf("PM: %f\n", readings.sen5x.pm2_5);
			CAT_gui_textf("NOX: %f\n", readings.sen5x.nox_index);
			CAT_gui_textf("VOC: %f\n", readings.sen5x.voc_index);
			CAT_gui_textf("TMP: %f%s\n", CAT_AQ_map_celsius(readings.lps22hh.temp), CAT_AQ_get_temperature_unit_string());
			CAT_gui_textf("AQI: %f\n", CAT_AQ_aggregate_score());
		}
		break;

		case PERSIST:
		{
			CAT_gui_title(true, "PERSIST");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

			CAT_AQ_crisis_state crisis;
			CAT_pet_timing_state timing;
			memcpy(&crisis, CAT_AQ_crisis_state_persist(), sizeof(crisis));
			memcpy(&timing, CAT_pet_timing_state_persist(), sizeof(timing));

			CAT_datetime life;
			CAT_make_datetime(timing.last_life_time, &life);
			CAT_gui_textf("Life: %d %d %d\n", life.year, life.month, life.day);
		}
		break;

		default:
		{
			CAT_gui_title(true, "LAST");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			CAT_gui_set_flag(CAT_GUI_FLAG_WRAPPED);
			CAT_gui_text("You shouldn't be here");
			break;
		}
	}	
}