#pragma once

#include "cat_math.h"
#include "cat_machine.h"

typedef struct CAT_deco_state
{
	enum mode {ADD, FLIP, REMOVE} mode;

	int add_id;
	CAT_rect add_rect;
	bool valid_add;

	int mod_idx;
	CAT_rect mod_rect;
} CAT_deco_state;
extern CAT_deco_state deco_state;

void CAT_MS_deco(CAT_machine_signal signal);
void CAT_render_deco(int cycle);