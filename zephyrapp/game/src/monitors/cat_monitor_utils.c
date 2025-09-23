#include "cat_monitor_utils.h"

#include "cat_monitors.h"
#include "cat_input.h"
#include "cat_render.h"
#include "cat_colours.h"
#include "cat_gui.h"
#include "cat_curves.h"
#include "cat_monitor_graphics_utils.h"

static const char* gate_title;
static float gate_progress;
static bool gate_lock = true;
static bool gate_initialized = false;

void CAT_monitor_gate_init(const char* title)
{
	gate_title = title;
	gate_progress = 0;
	gate_lock = true;
	gate_initialized = true;
}

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
	bool refresh = false;
	if(CAT_input_dismissal())
	{
		CAT_monitor_dismiss();
		refresh = true;
	}
	if(CAT_input_pressed(CAT_BUTTON_LEFT))
	{
		CAT_monitor_retreat();
		refresh = true;
	}
	if(CAT_input_pressed(CAT_BUTTON_RIGHT))
	{
		CAT_monitor_advance();
		refresh = true;
	}
	
	if(refresh)
	{
		gate_lock = true;
		gate_initialized = false;
	}

	if(CAT_input_down(CAT_BUTTON_A))
		gate_progress += CAT_get_delta_time_s();
	if(CAT_input_released(CAT_BUTTON_A))
	{
		if(gate_progress >= 1)
			gate_lock = false;
		else
			gate_progress = 0;
	}
	gate_progress = clamp(gate_progress, 0, 1.001f);
}

void CAT_monitor_gate_render()
{
	if(!gate_initialized)
		return;
	int cursor_y = center_textf(120, 60, 2, CAT_WHITE, gate_title);
	cursor_y = underline(120, cursor_y, 2, CAT_WHITE, gate_title);
	CAT_draw_lock(120, 200, 64, gate_progress, CAT_WHITE);
}

