#pragma once

#include "cat_core.h"

typedef enum CAT_button
{
    CAT_BUTTON_START,
    CAT_BUTTON_SELECT,
    CAT_BUTTON_A,
    CAT_BUTTON_B,
    CAT_BUTTON_DOWN,
    CAT_BUTTON_RIGHT,
    CAT_BUTTON_LEFT,
    CAT_BUTTON_UP,
    CAT_BUTTON_LAST
} CAT_button;

typedef struct CAT_input
{
	bool mask[CAT_BUTTON_LAST];
	bool last[CAT_BUTTON_LAST];
	float time[CAT_BUTTON_LAST];
	float since[CAT_BUTTON_LAST];
	float pulse[CAT_BUTTON_LAST];
	bool dirty[CAT_BUTTON_LAST];

	CAT_touch touch;
	CAT_touch touch_last;
	float touch_time;

	CAT_button buffer[10];
	int buffer_head;
} CAT_input;
extern CAT_input input;

void CAT_input_init();
void CAT_input_tick();
void CAT_input_clear();
void CAT_input_swallow(int button);

bool CAT_input_pressed(int button);
bool CAT_input_released(int button);
bool CAT_input_held(int button, float t);
bool CAT_input_pulse(int button);
float CAT_input_time(int button);

bool CAT_input_dismissal();

bool CAT_input_drag(int x, int y, float r);
bool CAT_input_touch(int x, int y, float r);
bool CAT_input_touch_rect(int x, int y, int w, int h);

bool CAT_input_touching();
bool CAT_input_touch_down();
bool CAT_input_touch_up();

CAT_ivec2 CAT_input_cursor();
bool CAT_input_cursor_in_rect(int x, int y, int w, int h);
bool CAT_input_cursor_in_circle(int x, int y, int r);

void CAT_input_buffer_clear();
bool CAT_input_spell(CAT_button* spell);

float CAT_input_time_since_last();
