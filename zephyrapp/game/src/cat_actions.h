#ifndef CAT_ACTION_H
#define CAT_ACTION_H

#include "cat_machine.h"
#include "cat_math.h"

typedef struct CAT_action_state
{
	CAT_machine_state action_MS;
	void (*action_proc)();
	CAT_ASM_state* action_AS;
	CAT_ASM_state* stat_up_AS;

	int item_id;
	CAT_vec2 location;
	bool confirmed;
	bool complete;
} CAT_action_state;
extern CAT_action_state action_state;

void CAT_action_state_init();
void CAT_action_tick();

#endif