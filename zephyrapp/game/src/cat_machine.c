#include "cat_machine.h"

#include <stddef.h>
#include "cat_core.h"
#include "cat_math.h"
#include "cat_render.h"
#include "cat_input.h"

//////////////////////////////////////////////////////////////////////////
// MACHINE

CAT_machine_state current = NULL;
CAT_machine_state next = NULL;
CAT_machine_state machine_stack[64];
int machine_depth = 0;

static void push(CAT_machine_state s)
{
	machine_stack[machine_depth] = s;
	machine_depth += 1;
}

static CAT_machine_state pop()
{
	machine_depth -= 1;
	return machine_stack[machine_depth];
}

static CAT_machine_state peek()
{
	return machine_stack[machine_depth-1];
}

void complete_transition(CAT_machine_state state)
{
	if(state == NULL)
	{
		CAT_printf("[ERROR] machine transition to NULL state\n");
		return;
	}

	if(current != NULL)
	{
		(current)(CAT_MACHINE_SIGNAL_EXIT);
	}

	bool loop_back = false;
	for(int i = 0; i < machine_depth; i++)
	{
		if(machine_stack[i] == state)
		{
			machine_depth = i+1;
			loop_back = true;
			break;
		}
	}
	if(!loop_back)
		push(state);

	current = state;
	(current)(CAT_MACHINE_SIGNAL_ENTER);
}

void CAT_machine_transition(CAT_machine_state state)
{
	next = state;
}

void CAT_machine_tick()
{
	if(next != NULL)
	{
		complete_transition(next);
		next = NULL;
	}
	if(current != NULL)
		(current)(CAT_MACHINE_SIGNAL_TICK);
}

void CAT_machine_back()
{
	if(machine_depth > 1)
	{
		pop();
		CAT_machine_transition(peek());
	}
}

CAT_machine_state CAT_get_machine_state()
{
	return current;
}

CAT_render_callback render_callback;

void CAT_set_render_callback(CAT_render_callback callback)
{
	render_callback = callback;
}

CAT_render_callback CAT_get_render_callback()
{
	return render_callback;
}
