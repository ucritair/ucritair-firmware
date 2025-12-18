#include "cat_monitor_utils.h"

#include "cat_monitors.h"
#include "cat_input.h"
#include "cat_render.h"
#include "cat_colours.h"
#include "cat_gui.h"
#include "cat_curves.h"
#include "cat_monitor_graphics_utils.h"

static float gate_progress;
static bool gate_lock = true;

bool CAT_monitor_gate_is_locked()
{
	return gate_lock;
}

bool CAT_monitor_gate_lock()
{
	gate_progress = 0;
	gate_lock = true;
}

void CAT_monitor_gate_logic()
{
	if(CAT_input_down(CAT_BUTTON_A))
		gate_progress += CAT_get_delta_time_s();
	if(CAT_input_released(CAT_BUTTON_A))
	{
		if(gate_progress >= 1)
			gate_lock = false;
		else
			gate_progress = 0;
	}
	gate_progress = CAT_clamp(gate_progress, 0, 1.001f);
}

void CAT_monitor_gate_draw(const char* title)
{
	int cursor_y = center_textf(120, 60, 2, CAT_WHITE, title);
	cursor_y = underline(120, cursor_y, 2, CAT_WHITE, title);
	CAT_draw_lock(120, 200, 64, gate_progress, CAT_WHITE);
}

