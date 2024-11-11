#pragma once

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_sprite.h"

typedef struct CAT_action_state
{
	CAT_machine_state action_MS;
	CAT_AM_state* action_AS;
	CAT_AM_state* stat_up_AS;

	int item_id;
	CAT_vec2 location;
	bool confirmed;
	bool complete;

	int timer_id;
} CAT_action_state;
extern CAT_action_state action_state;

extern void CAT_render_action(int cycle);