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
		input.dirty[i] = false;
	}

	input.touch.x = -1;
	input.touch.y = -1;
	input.touch.pressure = 0;
	input.touch_last = false;
	input.touch_time = 0;

	for(int i = 0; i < 10; i++)
		input.buffer[i] = CAT_BUTTON_LAST;
	input.buffer_head = 0;
}

int arbiter = 0;
int asker = 0;

void CAT_input_tick()
{
	uint16_t mask = CAT_get_buttons();

	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		bool current_state = (mask & (1 << i)) > 0;

		if(!current_state)
			input.dirty[i] = false;
		if(input.dirty[i])
			current_state = false;

		input.last[i] = input.mask[i];
		input.mask[i] = current_state;
		
		if(!input.mask[i])
			input.time[i] = 0;
		else
			input.time[i] += CAT_get_delta_time();

		if(input.mask[i] && !input.last[i])
		{
			input.buffer[input.buffer_head] = i;
			input.buffer_head += 1;
			if(input.buffer_head >= 10)
				input.buffer_head = 0;
		}
	}

	input.touch_last = input.touch.pressure > 0;
	CAT_get_touch(&input.touch);
	bool current_state = input.touch.pressure > 0;
	if(!current_state)
		input.touch_time = 0;
	else
		input.touch_time += CAT_get_delta_time();
}

void CAT_input_clear()
{
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		input.mask[i] = false;
		input.dirty[i] = true;
	}
}

bool CAT_input_enforce(int layer)
{
	if(layer > arbiter)
	{
		arbiter = layer;
		return true;
	}
	return false;
}

void CAT_input_ask(int layer)
{
	asker = layer;
}

void CAT_input_yield()
{
	arbiter = 0;
}

static bool arbitrate()
{
	return asker >= arbiter;
}

bool CAT_input_pressed(int button)
{
	if(!arbitrate())
		return false;

	return input.mask[button] && !input.last[button];
}

bool CAT_input_released(int button)
{
	if(!arbitrate())
		return false;

	return !input.mask[button] && input.last[button];
}

bool CAT_input_held(int button, float t)
{
	if(!arbitrate())
		return false;

	return input.mask[button] && input.time[button] >= t;
}

bool CAT_input_pulse(int button)
{
	if(!arbitrate())
		return false;

	if(input.mask[button] && input.time[button] < 0.15f)
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

float CAT_input_time(int button)
{
	return input.time[button];
}

bool CAT_input_drag(int x, int y, float r)
{
	if(!arbitrate())
		return false;

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

bool CAT_input_touch(int x, int y, float r)
{
	if(!arbitrate())
		return false;

	if(input.touch.pressure > 0 && !input.touch_last)
	{
		int x_t = input.touch.x;
		int y_t = input.touch.y;
		int x_d = x - x_t;
		int y_d = y - y_t;
		return x_d*x_d + y_d*y_d <= r*r;
	}
	return false;
}

bool CAT_input_touch_rect(int x, int y, int w, int h)
{
	if(!arbitrate())
		return false;

	if(input.touch.pressure <= 0 || input.touch_last)
		return false;
	if(input.touch.x < x || input.touch.x > (x + w))
		return false;
	if(input.touch.y < y || input.touch.y > (y + h))
		return false;
	return true;
}

bool CAT_input_touching()
{
	if(!arbitrate())
		return false;

	return input.touch.pressure;
}

void CAT_input_buffer_clear()
{
	for(int i = 0; i < 10; i++)
		input.buffer[i] = CAT_BUTTON_LAST;
	input.buffer_head = 0;
}

bool CAT_input_spell(CAT_button* spell)
{
	if(!arbitrate())
		return false;

	int i = (input.buffer_head+9) % 10;
	int steps = 0;
	while(steps < 10)
	{
		if(input.buffer[i] != spell[9-steps])
			return false;
		i -= 1;
		if(i < 0)
			i = 9;
		steps += 1;
	}
	return true;
}
