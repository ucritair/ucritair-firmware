#include "cat_input.h"

#include "cat_core.h"

CAT_input input;

void CAT_input_init()
{
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		input.mask[i] = false;
		input.last[i] = false;
		input.time[i] = 0;
	}
}

bool CAT_input_pressed(int button)
{
	return input.mask[button] && !input.last[button];
}

bool CAT_input_held(int button, float t)
{
	return input.mask[button] && input.time[button] >= t;
}

void CAT_input_tick(float dt)
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
			input.time[i] += dt;
	}
}
