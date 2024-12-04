#pragma once

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_sprite.h"
#include "cat_item.h"

typedef struct CAT_action_profile
{
	CAT_item_type tool_type;

	CAT_machine_state MS;
	CAT_animachine_state* AS;
	CAT_animachine_state* stat_AS;

	uint8_t LED_colour[3];
} CAT_action_profile;

typedef struct CAT_action_state
{
	CAT_action_profile* profile;

	int tool_id;
	CAT_vec2 location;
	bool confirmed;
	bool complete;

	int timer_id;
} CAT_action_state;
extern CAT_action_state action_state;

void CAT_MS_feed(CAT_machine_signal signal);
void CAT_MS_study(CAT_machine_signal signal);
void CAT_MS_play(CAT_machine_signal signal);
void CAT_render_action(int cycle);