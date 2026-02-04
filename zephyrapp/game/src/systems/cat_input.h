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

#define CAT_BUTTON_BIT(button) (1 << (button))
#define CAT_START_BIT (CAT_BUTTON_BIT(CAT_BUTTON_START))
#define CAT_SELECT_BIT (CAT_BUTTON_BIT(CAT_BUTTON_START))
#define CAT_A_BIT (CAT_BUTTON_BIT(CAT_BUTTON_A))
#define CAT_B_BIT (CAT_BUTTON_BIT(CAT_BUTTON_B))
#define CAT_DOWN_BIT (CAT_BUTTON_BIT(CAT_BUTTON_DOWN))
#define CAT_RIGHT_BIT (CAT_BUTTON_BIT(CAT_BUTTON_RIGHT))
#define CAT_LEFT_BIT (CAT_BUTTON_BIT(CAT_BUTTON_LEFT))
#define CAT_UP_BIT (CAT_BUTTON_BIT(CAT_BUTTON_UP))
#define CAT_TOUCH_BIT CAT_BUTTON_BIT(CAT_BUTTON_LAST)

bool CAT_input_down(int button);
bool CAT_input_held(int button, float t);
bool CAT_input_pressed(int button);
bool CAT_input_released(int button);
bool CAT_input_pulse(int button);

int CAT_input_frames(int button);
float CAT_input_time(int button);
float CAT_input_since(int button);

bool CAT_input_touching();
bool CAT_input_touch_down();
bool CAT_input_touch_up();

void CAT_input_cursor(int* x, int* y);
bool CAT_input_drag_circle(int x, int y, int r);
bool CAT_input_touch_circle(int x, int y, int r);
bool CAT_input_touch_rect(int x, int y, int w, int h);

float CAT_input_touch_time();

bool CAT_input_dismissal();

void CAT_input_buffer_clear();
int CAT_input_buffer_length();
int CAT_input_buffer_get(int idx);
bool CAT_input_spell(CAT_button* spell);

void CAT_input_raise_barrier(int mask);
bool CAT_input_poll_barrier();
void CAT_input_lower_barrier();

void CAT_input_init();
void CAT_input_tick();
void CAT_input_clear();
float CAT_input_downtime();