#include "cat_input.h"
#include "cat_core.h"
#include "cat_math.h"

CAT_input input;

void CAT_input_init()
{
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		input.mask[i] = false;
		input.last[i] = false;
		input.time[i] = 0;
		input.pulse[i] = 0;	
	}
}

void CAT_input_tick()
{
	uint16_t mask = CAT_get_buttons();

	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		bool old_state = input.last[i];
		input.last[i] = input.mask[i];
		input.mask[i] = (mask & (1 << i)) > 0;
		
		if(old_state != input.mask[i])
			input.time[i] = 0;
		else
			input.time[i] += CAT_get_delta_time();
	}

	CAT_get_touch(&input.touch);
}

bool CAT_input_pressed(int button)
{
	return input.mask[button] && !input.last[button];
}

bool CAT_input_released(int button)
{
	return !input.mask[button] && input.last[button];
}

bool CAT_input_held(int button, float t)
{
	return input.mask[button] && input.time[button] >= t;
}

bool CAT_input_pulse(int button)
{
	if(input.mask[button] && input.time[button] < 0.25f)
		return !input.last[button];

	bool pulse = false;
	if(input.mask[button])
	{
		if(input.pulse[button] == 0)
			pulse = true;
		input.pulse[button] += CAT_get_delta_time();
		if(input.pulse[button] >= 0.1f)
			input.pulse[button] = 0;
	}
	return pulse;
}

bool CAT_input_touch(int x, int y, float r)
{
	if(input.touch.pressure > 0)
	{
		int x_t = input.touch.x;
		int y_t = input.touch.y;
		int x_d = x - x_t;
		int y_d = y - y_t;
		return x_d*x_d + y_d*y_d <= r*r;
	}
	return false;
}

float CAT_input_progress(int button, float t)
{
	if(!input.mask[button])
		return 0;
	float progress = input.time[button] / t;
	return clampf(progress, 0, 1);
}

void CAT_input_clear(int button)
{
	input.mask[button] = false;
	input.last[button] = false;
	input.time[button] = 0;
	input.pulse[button] = 0;	
}

bool CAT_input_touch_rect(int x, int y, int w, int h)
{
	CAT_rect rect;
	rect.min.x = x;
	rect.min.y = y;
	rect.max.x = x + w;
	rect.max.y = y + h;
	CAT_ivec2 pt;
	pt.x = input.touch.x;
	pt.y = input.touch.y;
	return input.touch.pressure > 0 && CAT_rect_pt(pt, rect);
}
