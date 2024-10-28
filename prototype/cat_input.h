#ifndef CAT_INPUT_H
#define CAT_INPUT_H

#include "cat_core.h"

typedef struct CAT_input
{
	GLFWwindow* window;
	bool mask[CAT_BUTTON_LAST];
	bool last[CAT_BUTTON_LAST];
	float time[CAT_BUTTON_LAST];
} CAT_input;
extern CAT_input input;

void CAT_input_init();
bool CAT_input_pressed(int button);
bool CAT_input_held(int button, float t);
void CAT_input_tick(float dt);

#endif
