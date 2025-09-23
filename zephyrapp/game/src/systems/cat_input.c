#include "cat_input.h"

#include "cat_core.h"
#include "cat_math.h"

static bool mask[CAT_BUTTON_LAST];
static bool last[CAT_BUTTON_LAST];
static float time[CAT_BUTTON_LAST];
static float since[CAT_BUTTON_LAST];
static float pulse[CAT_BUTTON_LAST];
static uint32_t frames[CAT_BUTTON_LAST];
static bool dirty[CAT_BUTTON_LAST];

bool CAT_input_down(int button)
{
	return mask[button];
}

bool CAT_input_held(int button, float t)
{
	return mask[button] && time[button] >= t;
}

bool CAT_input_pressed(int button)
{
	return mask[button] && !last[button];
}

bool CAT_input_released(int button)
{
	return !mask[button] && last[button];
}

bool CAT_input_pulse(int button)
{
	float pulse_time = max(0.15, 2 * CAT_get_delta_time_s() + 0.005f);
	if(mask[button])
	{
		if(time[button] < pulse_time)
		{
			return !last[button];
		}
		return pulse[button] == 0;
	}
	return false;
}

int CAT_input_frames(int button)
{
	return frames[button];
}

float CAT_input_time(int button)
{
	return time[button];
}

float CAT_input_since(int button)
{
	return since[button];
}

static CAT_touch touch;
static CAT_touch touch_last;
static float touch_time;
static bool touch_dirty;

bool CAT_input_touching()
{
	return touch.pressure > 0;
}

bool CAT_input_touch_down()
{
	return touch.pressure > 0 && touch_last.pressure <= 0;
}

bool CAT_input_touch_up()
{
	return touch.pressure <= 0 && touch_last.pressure > 0;
}

void CAT_input_cursor(int* x, int* y)
{
	*x = touch.x;
	*y = touch.y;
}

bool CAT_input_drag_circle(int x, int y, int r)
{
	if(touch.pressure > 0)
	{
		int x_t = touch.x;
		int y_t = touch.y;
		int x_d = x - x_t;
		int y_d = y - y_t;
		return x_d*x_d + y_d*y_d <= r*r;
	}
	return false;
}

bool CAT_input_touch_circle(int x, int y, int r)
{
	if(touch.pressure > 0 && touch_last.pressure <= 0)
	{
		int x_t = touch.x;
		int y_t = touch.y;
		int x_d = x - x_t;
		int y_d = y - y_t;
		return x_d*x_d + y_d*y_d <= r*r;
	}

	return false;
}

bool CAT_input_touch_rect(int x, int y, int w, int h)
{
	if(touch.pressure <= 0 || touch_last.pressure > 0)
		return false;
	if(touch.x < x || touch.x > (x + w))
		return false;
	if(touch.y < y || touch.y > (y + h))
		return false;
	return true;
}

float CAT_input_touch_time()
{
	return touch_time;
}

bool CAT_input_dismissal()
{
	return
	CAT_input_pressed(CAT_BUTTON_B) ||
	CAT_input_pressed(CAT_BUTTON_START);
}

static uint8_t buffer[10];
static int buffer_head;

void CAT_input_buffer_clear()
{
	for(int i = 0; i < 10; i++)
		buffer[i] = CAT_BUTTON_LAST;
	buffer_head = 0;
}

bool CAT_input_spell(CAT_button* spell)
{
	int i = (buffer_head+9) % 10;
	int steps = 0;
	while(steps < 10)
	{
		if(buffer[i] != spell[9-steps])
			return false;
		i -= 1;
		if(i < 0)
			i = 9;
		steps += 1;
	}
	return true;
}

int CAT_input_buffer_head()
{
	return buffer_head;
}

int CAT_input_buffer_get(int idx)
{
	return buffer[idx];
}

static uint16_t barrier_mask = 0;
static bool barrier_up;

void CAT_input_raise_barrier(int mask)
{
	barrier_mask = mask;
	barrier_up = true;
}

bool CAT_input_poll_barrier()
{
	return barrier_up && barrier_mask != 0;
}

void CAT_input_lower_barrier()
{
	barrier_up = false;
}

static float downtime = 0;

void CAT_input_init()
{
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		mask[i] = false;
		last[i] = false;
		time[i] = 0;
		since[i] = 0;
		pulse[i] = 0;
		frames[i] = 0;
		dirty[i] = false;
	}
	downtime = 0;

	touch.x = 0;
	touch.y = 0;
	touch.pressure = 0;
	touch_last = (CAT_touch) 
	{
		.x = 0,
		.y = 0,
		.pressure = 0
	};
	touch_time = 0;

	for(int i = 0; i < 10; i++)
		buffer[i] = CAT_BUTTON_LAST;
	buffer_head = 0;
}

uint16_t swap_mask_bits(uint16_t mask, int a_idx, int b_idx)
{
	uint16_t x = ((mask >> a_idx) ^ (mask >> b_idx)) & 1;
	return ((x << a_idx) | (x << b_idx)) ^ mask;
}

void tick_buttons()
{
	const float dt = CAT_get_delta_time_s();

	uint16_t raw = CAT_get_buttons();
	if(CAT_get_screen_orientation() == CAT_SCREEN_ORIENTATION_DOWN)
	{
		raw = swap_mask_bits(raw, CAT_BUTTON_UP, CAT_BUTTON_DOWN);
		raw = swap_mask_bits(raw, CAT_BUTTON_LEFT, CAT_BUTTON_RIGHT);
		raw = swap_mask_bits(raw, CAT_BUTTON_A, CAT_BUTTON_B);
		raw = swap_mask_bits(raw, CAT_BUTTON_START, CAT_BUTTON_SELECT);
	}
	
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		bool now = (raw & (1 << i)) > 0;

		if(!now)
			dirty[i] = false;
		if(dirty[i])
			now = false;

		last[i] = mask[i];
		mask[i] = now;
		
		if(!mask[i])
		{
			time[i] = 0;
			since[i] += dt;
			pulse[i] = 0;
			frames[i] = 0;
		}
		else
		{
			time[i] += dt;
			since[i] = 0;
			pulse[i] += dt;
			if(pulse[i] >= 0.1f)
				pulse[i] = 0;
			frames[i] += 1;
		}
	}
}

void tick_touch()
{
	const float dt = CAT_get_delta_time_s();

	touch_last = touch;
	CAT_get_touch(&touch);
	if(CAT_get_screen_orientation() == CAT_SCREEN_ORIENTATION_DOWN)
	{
		touch.x = CAT_LCD_SCREEN_W - touch.x - 1;
		touch.y = CAT_LCD_SCREEN_H - touch.y - 1;
	}
	touch_time = touch.pressure > 0 ? touch_time + dt : 0;

	if(touch.pressure == 0)
		touch_dirty = false;
	if(touch_dirty)
		touch.pressure = 0;
}

void tick_buffer()
{
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		if(mask[i] && !last[i])
		{
			buffer[buffer_head] = i;
			buffer_head += 1;
			if(buffer_head >= 10)
				buffer_head = 0;
		}
	}
}

void tick_barrier()
{
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		if(mask[i] && !last[i])
			barrier_mask &= ~(1 << i);
	}
	if(CAT_input_touching())
		barrier_mask &= ~(1 << CAT_BUTTON_LAST);
	if(barrier_up && barrier_mask == 0)
	{
		barrier_up = false;
		CAT_input_clear();
	}
}

void CAT_input_tick()
{
	tick_buttons();
	tick_touch();
	tick_buffer();
	tick_barrier();

	downtime += CAT_get_delta_time_s();
	if(CAT_get_buttons() > 0)
		downtime = 0;
}

void CAT_input_clear()
{
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		mask[i] = false;
		dirty[i] = true;
	}
	touch.pressure = 0;
	touch_dirty = true;
}

float CAT_input_downtime()
{
	return downtime;
}