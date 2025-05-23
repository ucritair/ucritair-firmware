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

	input.touch.x = 0;
	input.touch.y = 0;
	input.touch.pressure = 0;
	input.touch_last = (CAT_touch) 
	{
		.x = 0,
		.y = 0,
		.pressure = 0
	};
	input.touch_time = 0;

	for(int i = 0; i < 10; i++)
		input.buffer[i] = CAT_BUTTON_LAST;
	input.buffer_head = 0;
}

static float time_since_last_input = 0.0f;
static bool input_this_frame = false;

uint16_t swap_mask_bits(uint16_t mask, int a_idx, int b_idx)
{
	uint16_t x = ((mask >> a_idx) ^ (mask >> b_idx)) & 1;
	return ((x << a_idx) | (x << b_idx)) ^ mask;
}

void CAT_input_tick()
{
	uint16_t mask = CAT_get_buttons();
	if(CAT_get_screen_orientation() == CAT_SCREEN_ORIENTATION_DOWN)
	{
		mask = swap_mask_bits(mask, CAT_BUTTON_UP, CAT_BUTTON_DOWN);
		mask = swap_mask_bits(mask, CAT_BUTTON_LEFT, CAT_BUTTON_RIGHT);
		mask = swap_mask_bits(mask, CAT_BUTTON_A, CAT_BUTTON_B);
		mask = swap_mask_bits(mask, CAT_BUTTON_START, CAT_BUTTON_SELECT);
	}
	
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
		{
			input.time[i] = 0;
			input.pulse[i] = 0;
		}
		else
		{
			input.time[i] += CAT_get_delta_time_s();
			input.pulse[i] += CAT_get_delta_time_s();
			if(input.pulse[i] >= 0.1f)
				input.pulse[i] = 0;
		}

		if(input.mask[i] && !input.last[i])
		{
			input.buffer[input.buffer_head] = i;
			input.buffer_head += 1;
			if(input.buffer_head >= 10)
				input.buffer_head = 0;
		}
	}

	input.touch_last = input.touch;
	CAT_get_touch(&input.touch);
	bool current_state = input.touch.pressure > 0;
	if(!current_state)
		input.touch_time = 0;
	else
		input.touch_time += CAT_get_delta_time_s();
	if(CAT_get_screen_orientation() == CAT_SCREEN_ORIENTATION_DOWN)
	{
		input.touch.x = CAT_LCD_SCREEN_W - input.touch.x - 1;
		input.touch.y = CAT_LCD_SCREEN_H - input.touch.y - 1;
	}

	input_this_frame = false;
	input_this_frame |= mask > 0;
	input_this_frame |= input.touch.pressure > 0;
	if(!input_this_frame)
		time_since_last_input += CAT_get_delta_time_s();
	else
		time_since_last_input = 0;
}

void CAT_input_clear()
{
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		input.mask[i] = false;
		input.dirty[i] = true;
	}
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
	float pulse_time = maxf(0.15, 2 * CAT_get_delta_time_s() + 0.005f);

	if(input.mask[button])
	{
		if(input.time[button] < pulse_time)
		{
			return !input.last[button];
		}
		return input.pulse[button] == 0;
	}
	return false;
}

float CAT_input_time(int button)
{
	return input.time[button];
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
	if(input.touch.pressure > 0 && input.touch_last.pressure <= 0)
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
	if(input.touch.pressure <= 0 || input.touch_last.pressure > 0)
		return false;
	if(input.touch.x < x || input.touch.x > (x + w))
		return false;
	if(input.touch.y < y || input.touch.y > (y + h))
		return false;
	return true;
}

bool CAT_input_touching()
{
	return input.touch.pressure > 0;
}

bool CAT_input_touch_down()
{
	return input.touch.pressure > 0 && input.touch_last.pressure <= 0;
}

bool CAT_input_touch_up()
{
	return input.touch.pressure <= 0 && input.touch_last.pressure > 0;
}

CAT_ivec2 CAT_input_cursor()
{
	return (CAT_ivec2) {input.touch.x, input.touch.y};
}

bool CAT_input_cursor_in_rect(int x, int y, int w, int h)
{
	if(input.touch.x < x || input.touch.x > x + w)
		return false;
	if(input.touch.y < y || input.touch.y > y + h)
		return false;
	return true;
}

bool CAT_input_cursor_in_circle(int x, int y, int r)
{
	int dx = input.touch.x - x;
	int dy = input.touch.y - y;
	return dx*dx + dy*dy < r*r;
}

void CAT_input_buffer_clear()
{
	for(int i = 0; i < 10; i++)
		input.buffer[i] = CAT_BUTTON_LAST;
	input.buffer_head = 0;
}

bool CAT_input_spell(CAT_button* spell)
{
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

float CAT_input_time_since_last()
{
	return time_since_last_input;
}
