#include "cat_menu.h"

#include "cat_input.h"
#include "cat_version.h"
#include "cat_gui.h"
#include "cat_main.h"
#include "cat_pet.h"
#include "cat_room.h"
#include <stdint.h>
#include "cat_air.h"
#include "sprite_assets.h"
#include "cat_crisis.h"
#include "cat_persist.h"
#include "cat_time.h"
#include "cat_save.h"

enum
{
	SYSTEM,
	TIME,
	AQI,
	LOGS,
	LAST
};
static int page = SYSTEM;

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

#define PAD 8
static int cursor_x = PAD;
static int cursor_y = PAD;

static void draw_page(const char* title)
{
	cursor_x = PAD;
	cursor_y = PAD;

	CAT_frameberry(CAT_WHITE);
	cursor_y = CAT_draw_textf(cursor_x, cursor_y, "%s\n", title);
	cursor_y += PAD;
	CAT_rowberry(cursor_y, cursor_y+1, CAT_BLACK);
	cursor_y += PAD;
}

void CAT_render_debug()
{
	switch(page)
	{
		case SYSTEM:
		{
			draw_page("< SYSTEM >");

			cursor_y = CAT_draw_textf
			(
				cursor_x, cursor_y,
				"Game v%d.%d.%d.%d\n",
				CAT_VERSION_MAJOR, CAT_VERSION_MINOR,
				CAT_VERSION_PATCH, CAT_VERSION_PUSH
			);
			
#if defined(CAT_DESKTOP)
			cursor_y = CAT_draw_text(cursor_x, cursor_y, "[Desktop]\n");
#elif defined(CAT_EMBEDDED)
			cursor_y = CAT_draw_text(cursor_x, cursor_y, "[Embedded]\n");
#endif

			if(CAT_check_config_flags(CAT_SAVE_CONFIG_FLAG_DEVELOPER))
				cursor_y = CAT_draw_text(cursor_x, cursor_y, "[Developer mode]\n");

			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Debug number: %d\n", CAT_get_debug_number());
		}
		break;
		
		case TIME:
		{
			draw_page("< TIME >");

			CAT_datetime today;
			CAT_get_datetime(&today);
			cursor_y = CAT_draw_textf
			(
				cursor_x, cursor_y,
				"%.2d/%.2d/%.4d %.2d:%.2d:%.2d\n",
				today.month, today.day, today.year, today.hour, today.minute, today.second
			);

			CAT_make_datetime(pet_timing_state.last_life_time, &today);
			cursor_y = CAT_draw_textf
			(
				cursor_x, cursor_y,
				"Life: %d/%d/%d %d:%d:%d\n",
				today.year, today.month, today.day, today.hour, today.minute, today.second
			);
			CAT_make_datetime(pet_timing_state.last_stat_time, &today);
			cursor_y = CAT_draw_textf
			(
				cursor_x, cursor_y,
				"Stat: %d/%d/%d %d:%d:%d\n",
				today.year, today.month, today.day, today.hour, today.minute, today.second
			);

			uint64_t now = CAT_get_RTC_now();
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Since Life: %llus/%ds\n", now-pet_timing_state.last_life_time, CAT_LIFE_TICK_PERIOD);
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Since Stat: %llus/%ds\n", now-pet_timing_state.last_stat_time, CAT_STAT_TICK_PERIOD);

			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Slept: %llus\n", CAT_get_slept_s());
		}
		break;

		case AQI:
		{
			draw_page("< AIR QUALITY >");

			if(CAT_AQ_sensors_initialized())
				cursor_y = CAT_draw_text(cursor_x, cursor_y, "Sensors initialized\n");

			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "RH: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(readings.sen5x.humidity_rhpct));
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "CO2: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(readings.sunrise.ppm_filtered_compensated));
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "PM: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(readings.sen5x.pm2_5));
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "NOX: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(readings.sen5x.nox_index));
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "VOC: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(readings.sen5x.voc_index));
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "TMP: " CAT_FLOAT_FMT "%s\n", CAT_FMT_FLOAT(CAT_AQ_map_celsius(readings.lps22hh.temp)), CAT_AQ_get_temperature_unit_string());
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "AQI: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(CAT_AQ_aggregate_score()));
		}
		break;

		case LOGS:
		{
			draw_page("< LOGS >");

			if(CAT_AQ_sensors_initialized())
				cursor_y = CAT_draw_text(cursor_x, cursor_y, "Logs initialized\n");

			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Log count: %d\n", CAT_get_log_cell_count());
			
			CAT_log_cell cell;
			int cell_idx = CAT_read_first_calendar_cell(&cell);
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Earliest log: %d\n", cell_idx);

			CAT_datetime cell_time;
			CAT_make_datetime(cell.timestamp, &cell_time);
			cursor_y = CAT_draw_textf
			(
				cursor_x, cursor_y,
				"(%.4d/%.2d/%.2d)\n",
				cell_time.year, cell_time.month, cell_time.day
			);
		}
		break;

		default:
		{
			draw_page("INVALID");
			break;
		}
	}	
}