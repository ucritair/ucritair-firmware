#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include <time.h>
#include <math.h>
#include "sprite_assets.h"

#define M_PI_6 0.52359877559

#define CLOCK_X 120
#define CLOCK_Y 140
#define CLOCK_RADIUS 96
#define CLOCK_HOUR_HAND_LENGTH (CLOCK_RADIUS * 0.6f)
#define CLOCK_MINUTE_HAND_LENGTH (CLOCK_RADIUS * 0.75f)
#define CLOCK_SECOND_HAND_LENGTH (CLOCK_RADIUS * 0.9f)

static CAT_datetime now;
static float hour_arc;
static float minute_arc;
static float second_arc;

static CAT_vec2 radial_point(float t, float r)
{
	float dx = cos(t) * r;
	float dy = sin(t) * r;
	float x = CLOCK_X + dx;
	float y = CLOCK_Y - dy;
	return (CAT_vec2) {x, y};
}

void CAT_monitor_render_clock()
{
	CAT_set_sprite_flags(CAT_DRAW_FLAG_BOTTOM);
	CAT_draw_sprite(&monitor_clouds_sprite, 0, 0, 320);
	
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(8, 30, "%.2i:%.2i:%.2is", now.hour, now.minute, now.second);

	CAT_circberry(CLOCK_X, CLOCK_Y, CLOCK_RADIUS, CAT_WHITE);
	CAT_circberry(CLOCK_X, CLOCK_Y, CLOCK_RADIUS+4, CAT_WHITE);

	for(int i = 1; i <= 12; i++)
	{
		float t = M_PI_6 + i * M_PI_6;
		CAT_vec2 a = radial_point(t, CLOCK_RADIUS * 0.9f);
		CAT_vec2 b = radial_point(t, CLOCK_RADIUS * 0.95f);
		CAT_lineberry(a.x, a.y, b.x, b.y, CAT_WHITE);
	}

	CAT_vec2 hour_point = radial_point(hour_arc, CLOCK_HOUR_HAND_LENGTH);
	CAT_lineberry(CLOCK_X, CLOCK_Y, hour_point.x, hour_point.y, CAT_WHITE);

	CAT_vec2 minute_point = radial_point(minute_arc, CLOCK_MINUTE_HAND_LENGTH);
	CAT_lineberry(CLOCK_X, CLOCK_Y, minute_point.x, minute_point.y, CAT_WHITE);

	CAT_vec2 second_point = radial_point(second_arc, CLOCK_SECOND_HAND_LENGTH);
	CAT_lineberry(CLOCK_X, CLOCK_Y, second_point.x, second_point.y, CAT_WHITE);
}

static void clock_tick()
{
	CAT_get_datetime(&now);

	float arc_base = M_PI * 0.5f;

	float hour_t = now.hour / 24.0f;
	float min_t = now.minute / 60.0f;
	float sec_t = now.second / 60.0f;

	hour_arc = arc_base - hour_t * (4 * M_PI);
	minute_arc = arc_base - min_t * (2 * M_PI);
	second_arc = arc_base - sec_t * (2 * M_PI);
}

void CAT_monitor_MS_clock(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_dismissal())
				CAT_monitor_soft_exit();
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				CAT_monitor_retreat();
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				CAT_monitor_advance();
			
			clock_tick();
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}