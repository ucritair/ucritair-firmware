#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "cat_aqi.h"
#include "mesh2d_assets.h"
#include "sprite_assets.h"

void CAT_monitor_render_gameplay()
{
	if(CAT_AQ_is_crisis_ongoing())
	{
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(12, 44, CAT_AQ_get_crisis_title());
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(12, 44 + 26, CAT_AQ_get_crisis_severity_string());
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(12, 44 + 26 + 14, "%d / %d\n", CAT_AQ_get_crisis_duration(), CAT_AQ_get_crisis_primetime());
	}
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
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}