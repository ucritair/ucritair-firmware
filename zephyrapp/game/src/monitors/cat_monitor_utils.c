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

void CAT_monitor_gate_init(const char* title)
{
	gate_title = title;
	gate_progress = 0;
}

bool CAT_monitor_gate_is_locked()
{
	return gate_progress < 1.0f;
}

bool CAT_monitor_gate_lock()
{
	gate_progress = 0;
}

void CAT_monitor_gate_logic()
{
	if(CAT_input_dismissal())
		CAT_monitor_dismiss();
	if(CAT_input_pressed(CAT_BUTTON_LEFT))
		CAT_monitor_retreat();
	if(CAT_input_pressed(CAT_BUTTON_RIGHT))
		CAT_monitor_advance();

	if(CAT_input_held(CAT_BUTTON_A, 0))
		gate_progress += CAT_get_delta_time_s();
	if(CAT_input_released(CAT_BUTTON_A))
		gate_progress = 0;
	gate_progress = clamp(gate_progress, 0, 1.001f);
}

void CAT_monitor_gate_render()
{
	int cursor_y = center_textf(120, 60, 2, CAT_WHITE, gate_title);
	cursor_y = underline(120, cursor_y, 2, CAT_WHITE, gate_title);

	CAT_annulusberry(120, 200, 64, 56, CAT_WHITE, CAT_ease_inout_sine(gate_progress), 0.25);
	CAT_circberry(120, 200, 56, CAT_WHITE);
	CAT_circberry(120, 200, 64, CAT_WHITE);

	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_scale(3);
	CAT_draw_text(120, 200-18, "A");
}

