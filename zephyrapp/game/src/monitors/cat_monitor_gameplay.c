#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "cat_aqi.h"
#include "mesh2d_assets.h"
#include "sprite_assets.h"

#define BANNER_Y 40

#define SKULLS_X 12
#define SKULLS_Y 80

static bool crisis = false;

static void draw_crisis_alert()
{
	if(crisis)
	{
		CAT_draw_mesh2d(&crisis_alert_mesh2d, 0, BANNER_Y, CAT_RED);
		CAT_set_text_colour(CAT_RED);
		if(CAT_pulse(0.25f))
			CAT_draw_text(8, BANNER_Y + 8, "CRISIS ALERT!");
	}
	else
	{
		CAT_draw_mesh2d(&crisis_alert_mesh2d, 0, BANNER_Y, CAT_WHITE);
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(8, BANNER_Y + 8, "ALL CLEAR");
	}
}

void CAT_monitor_render_gameplay()
{
	draw_crisis_alert();
}

void CAT_monitor_MS_gameplay(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_released(CAT_BUTTON_START))
				CAT_monitor_exit();
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				CAT_monitor_retreat();
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				CAT_monitor_advance();

			if(CAT_input_touch_down())
				crisis = !crisis;
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}