#pragma once

#include "cat_core.h"

typedef struct CAT_input
{
	bool mask[CAT_BUTTON_LAST];
	bool last[CAT_BUTTON_LAST];
	float time[CAT_BUTTON_LAST];
	float pulse[CAT_BUTTON_LAST];
	bool dirty[CAT_BUTTON_LAST];

	CAT_touch touch;
	bool touch_last;
	float touch_time;

	CAT_button buffer[10];
	int buffer_head;
} CAT_input;
extern CAT_input input;

void CAT_input_init();
void CAT_input_tick();
void CAT_input_clear();

bool CAT_input_commandeer(int layer);
void CAT_input_ask(int layer);
void CAT_input_yield();

bool CAT_input_pressed(int button);
bool CAT_input_released(int button);

bool CAT_input_held(int button, float t);
bool CAT_input_pulse(int button);

bool CAT_input_drag(int x, int y, float r);
bool CAT_input_touch(int x, int y, float r);
bool CAT_input_touch_rect(int x, int y, int w, int h);
bool CAT_input_touching();

void CAT_input_buffer_clear();
bool CAT_input_spell(CAT_button* spell);
