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

	input.touch.x = -1;
	input.touch.y = -1;
	input.touch.pressure = 0;
	input.touch_last = false;
	input.touch_time = 0;

	for(int i = 0; i < 10; i++)
		input.buffer[i] = CAT_BUTTON_LAST;
	input.buffer_head = 0;
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

		if(input.mask[i] && !input.last[i])
		{
			input.buffer[input.buffer_head] = i;
			input.buffer_head += 1;
			if(input.buffer_head >= 10)
				input.buffer_head = 0;
		}
	}

	bool old_state = input.touch_last;
	input.touch_last = input.touch.pressure > 0;
	CAT_get_touch(&input.touch);
	if((input.touch.pressure > 0) != old_state)
		input.touch_time = 0;
	else
		input.touch_time += CAT_get_delta_time();
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

bool CAT_input_drag(int x, int y, float r)
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

bool CAT_input_touch(int x, int y, float r)
{
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
	return input.touch.pressure;
}

static CAT_button konami_code[10] =
{
	CAT_BUTTON_UP,
	CAT_BUTTON_UP,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_B,
	CAT_BUTTON_A
};

bool CAT_konami()
{
	int i = (input.buffer_head+9) % 10;
	int steps = 0;
	while(steps < 10)
	{
		if(input.buffer[i] != konami_code[9-steps])
			return false;
		i -= 1;
		if(i < 0)
			i = 9;
		steps += 1;
	}
	return true;
}
