#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "cat_aqi.h"
#include "sprite_assets.h"
#include <math.h>

enum
{
	SUMMARY,
	GRAPH,
	CLOCK,
	PAGE_MAX
};
static int page = SUMMARY;

static void render_page_markers(int x, int y)
{
	int start_x = x - ((16 + 2) * PAGE_MAX) / 2;
	for(int i = 0; i < PAGE_MAX; i++)
	{
		int x = start_x + i * (16 + 2);
		CAT_draw_sprite(&ui_radio_button_diamond_sprite, page == i, x, y);
	}
}

static void render_summary()
{
	int cursor_y = 32;	
	CAT_push_text_scale(2);
	CAT_push_text_colour(CAT_WHITE);
	CAT_draw_text(12, cursor_y, "Air Quality");
	cursor_y += 48;

	if(!CAT_is_AQ_initialized())
	{
		CAT_push_text_scale(2);
		CAT_push_text_flags(CAT_TEXT_FLAG_WRAP);
		CAT_push_text_line_width(CAT_LCD_SCREEN_W-24);
		CAT_push_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_text(12, cursor_y, "Air quality sensors are coming online. Please wait...");
	}
	else
	{
		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "CO2  %.0f ppm", readings.sunrise.ppm_filtered_compensated);
		cursor_y += 14;

		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "PM2.5  %.0f \4g/m\5", readings.sen5x.pm2_5);
		cursor_y += 14;

		if (readings.sen5x.nox_index && readings.sen5x.voc_index)
		{
			CAT_push_text_colour(CAT_WHITE);
			CAT_draw_textf(12, cursor_y, "NOX %.0f / VOC %.0f", readings.sen5x.nox_index, readings.sen5x.voc_index);
			cursor_y += 14;
		}

		float deg_c = readings.sen5x.temp_degC;
		float deg_mapped = CAT_AQ_map_celsius(deg_c);
		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "%.0f %s / %.0f%% RH", deg_mapped, CAT_AQ_get_temperature_unit_string(), readings.sen5x.humidity_rhpct);
		cursor_y += 28;

		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "%.1f%% rebreathed", ((((double) readings.sunrise.ppm_filtered_compensated)-420.)/38000.)*100.);
		cursor_y += 14;

		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "uCritAQI %.1f%%", CAT_AQI_aggregate());
		cursor_y += 14;
		
		CAT_datetime t;
		CAT_get_datetime(&t);
		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "at %2d:%02d:%02d", t.hour, t.minute, t.second);
		cursor_y += 14;
	}
}

static void render_graph()
{
	CAT_push_text_colour(CAT_WHITE);
	CAT_draw_text(12, 24, "GRAPH");
	CAT_strokeberry(12, 38, CAT_LCD_SCREEN_W-24, 128, CAT_WHITE);
}

#define CLOCK_X 120
#define CLOCK_Y 132
#define CLOCK_RADIUS 96
#define CLOCK_HOUR_HAND_LENGTH (CLOCK_RADIUS * 0.5f)
#define CLOCK_MINUTE_HAND_LENGTH (CLOCK_RADIUS * 0.7f)
#define CLOCK_SECOND_HAND_LENGTH (CLOCK_RADIUS * 0.85f)

struct
{
	CAT_datetime datetime;
	float hour_arc;
	float minute_arc;
	float second_arc;
} clock;

static void clock_tick()
{
	CAT_get_datetime(&clock.datetime);

	float arc_base = M_PI * 0.5f;
	float arc_full = 4 * M_PI;

	float hour_t = clock.datetime.hour / 24.0f;
	float min_t = clock.datetime.minute / 60.0f;
	float sec_t = clock.datetime.second / 60.0f;

	clock.hour_arc = arc_base - hour_t * arc_full;
	clock.minute_arc = arc_base - min_t * arc_full;
	clock.second_arc = arc_base - sec_t * arc_full;
}

CAT_vec2 clock_point(float t, float r)
{
	float dx = cos(t) * r;
	float dy = sin(t) * r;
	float x = CLOCK_X + dx;
	float y = CLOCK_Y + dy;
	return (CAT_vec2) {x, y};
}

static void render_clock()
{
	CAT_push_text_colour(CAT_WHITE);
	CAT_draw_textf(12, 24, "%d:%d\n%ds", clock.datetime.hour, clock.datetime.minute, clock.datetime.second);

	CAT_circberry(120, 132, 96, CAT_WHITE);

	CAT_vec2 hour_point = clock_point(clock.hour_arc, CLOCK_HOUR_HAND_LENGTH);
	CAT_lineberry(CLOCK_X, CLOCK_Y, hour_point.x, hour_point.y, CAT_WHITE);

	CAT_vec2 minute_point = clock_point(clock.minute_arc, CLOCK_MINUTE_HAND_LENGTH);
	CAT_lineberry(CLOCK_X, CLOCK_Y, minute_point.x, minute_point.y, CAT_WHITE);

	CAT_vec2 second_point = clock_point(clock.second_arc, CLOCK_SECOND_HAND_LENGTH);
	CAT_lineberry(CLOCK_X, CLOCK_Y, second_point.x, second_point.y, CAT_WHITE);
}

static void render_monitor_air()
{
	CAT_frameberry(RGB8882565(35, 157, 235));
	render_page_markers(120, 8);

	CAT_discberry(-32, 320, 116, RGB8882565(220, 220, 220));
	CAT_discberry(92, 320, 48, CAT_WHITE);
	CAT_discberry(200, 320, 92, CAT_WHITE);

	CAT_push_text_colour(RGB8882565(35, 157, 235));
	CAT_draw_text(64, 304, "[START] to enter game");

	switch (page)
	{
		case SUMMARY:
			render_summary();
		break;

		case GRAPH:
			render_graph();
		break;

		case CLOCK:
			render_clock();
		break;
	}
}

void CAT_MS_monitor_air(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_monitor_air);
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

			if (CAT_input_pressed(CAT_BUTTON_RIGHT))
				page += 1;
			if (CAT_input_pressed(CAT_BUTTON_LEFT))
				page -= 1;
			page = (page + PAGE_MAX) % PAGE_MAX;

			switch (page)
			{
				case SUMMARY:
				break;

				case GRAPH:
				break;

				case CLOCK:
					clock_tick();
				break;
			}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}