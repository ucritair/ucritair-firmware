#ifndef CAT_DECO_H
#define CAT_DECO_H

#include "cat_math.h"

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

void CAT_deco_state_init();
extern void CAT_render_deco(int cycle);

#endif