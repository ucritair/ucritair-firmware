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
#include "cat_persist.h"
#include "cat_time.h"

static enum
{
	SYSTEM,
	TIME,
	DECO,
	INPUT,
	AQI,
	PERSIST,
	LOGS,
	LAST
};
int page = SYSTEM;

void CAT_MS_debug(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_debug);
			break;
		case CAT_FSM_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_pushdown_pop();

			if(CAT_input_pulse(CAT_BUTTON_LEFT))
				page -= 1;
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
				page += 1;
			page = CAT_wrap(page, LAST);

			break;
		case CAT_FSM_SIGNAL_EXIT:
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

			if(CAT_check_config_flags(CAT_SAVE_CONFIG_FLAG_DEVELOPER))
				CAT_gui_text("DEVELOPER MODE\n");
			if(is_persist_fresh)
				CAT_gui_text("PERSIST RESET\n");

			CAT_gui_textf("DEBUG NUMBER: %d\n", CAT_get_debug_number());
		break;
		
		case TIME:
			CAT_gui_title(true, "TIME");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

			CAT_datetime today;
			CAT_get_datetime(&today);
			CAT_gui_textf("%.2d/%.2d/%.4d %.2d:%.2d:%.2d\n", today.month, today.day, today.year, today.hour, today.minute, today.second);

			CAT_pet_timing_state pet_timing;
			CAT_pet_export_timing_state(&pet_timing);
			
			CAT_make_datetime(pet_timing.last_life_time, &today);
			CAT_gui_textf("Life: %d/%d/%d %d:%d:%d\n", today.year, today.month, today.day, today.hour, today.minute, today.second);
			CAT_make_datetime(pet_timing.last_stat_time, &today);
			CAT_gui_textf("Stat: %d/%d/%d %d:%d:%d\n", today.year, today.month, today.day, today.hour, today.minute, today.second);

			uint64_t now = CAT_get_RTC_now();
			CAT_gui_textf("Since Life: %llus/%ds\n", now-pet_timing.last_life_time, CAT_LIFE_TICK_PERIOD);
			CAT_gui_textf("Since Stat: %llus/%ds\n", now-pet_timing.last_stat_time, CAT_STAT_TICK_PERIOD);

			CAT_gui_textf("Slept: %llus\n", CAT_get_slept_s());

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

			CAT_gui_textf("Downtime: %d\n", (int) CAT_input_downtime());

			CAT_gui_textf("Touching: %s\n", CAT_input_touching() ? "Y" : "N");
			if(CAT_input_touching())
			{
				int x, y;
				CAT_input_cursor(&x, &y);
				CAT_gui_textf("(%d, %d)\n", x, y);
			}
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

			CAT_gui_textf("RH: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(readings.sen5x.humidity_rhpct));
			CAT_gui_textf("CO2: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(readings.sunrise.ppm_filtered_compensated));
			CAT_gui_textf("PM: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(readings.sen5x.pm2_5));
			CAT_gui_textf("NOX: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(readings.sen5x.nox_index));
			CAT_gui_textf("VOC: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(readings.sen5x.voc_index));
			CAT_gui_textf("TMP: " CAT_FLOAT_FMT "%s\n", CAT_FMT_FLOAT(CAT_AQ_map_celsius(readings.lps22hh.temp)), CAT_AQ_get_temperature_unit_string());
			CAT_gui_textf("AQI: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(CAT_AQ_aggregate_score()));
		}
		break;

		case PERSIST:
		{
			CAT_gui_title(true, "PERSIST");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

			CAT_datetime life;
			CAT_make_datetime(pet_timing_state.last_life_time, &life);
			CAT_gui_textf("Life: %d %d %d\n", life.year, life.month, life.day);
		}
		break;

		case LOGS:
		{
			CAT_gui_title(true, "LOGS");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

			CAT_gui_textf("Next log index: %d\n", CAT_get_log_cell_count());
			
			CAT_log_cell cell;
			int cell_idx = CAT_read_first_calendar_cell(&cell);
			CAT_gui_textf("First valid log index: %d\n", cell_idx);
			CAT_datetime cell_time;
			CAT_make_datetime(cell.timestamp, &cell_time);
			CAT_gui_textf("(%.4d/%.2d/%.2d)\n", cell_time.year, cell_time.month, cell_time.day);
			CAT_gui_textf("%d %d %d %d\n", cell.temp_Cx1000/1000, cell.co2_ppmx1, cell.voc_index, cell.nox_index);
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