#include "cat_machine.h"

#include <stddef.h>
#include "cat_core.h"
#include "cat_math.h"
#include "cat_render.h"
#include "cat_input.h"

//////////////////////////////////////////////////////////////////////////
// MACHINE

void CAT_FSM_transition(CAT_FSM* machine, CAT_FSM_state state)
{
	if(machine == NULL || state == NULL)
		return;

	if(machine->state != NULL)
		(*machine->state)(CAT_FSM_SIGNAL_EXIT);

	machine->state = state;
	(*machine->state)(CAT_FSM_SIGNAL_ENTER);
}

void CAT_FSM_tick(CAT_FSM* machine)
{
	(*machine->state)(CAT_FSM_SIGNAL_TICK);
}

CAT_FSM_state next = NULL;
CAT_FSM_state machine_stack[64];
int machine_depth = 0;

static void push(CAT_FSM_state s)
{
	machine_stack[machine_depth] = s;
	machine_depth += 1;
}

static CAT_FSM_state pop()
{
	machine_depth -= 1;
	return machine_stack[machine_depth];
}

static CAT_FSM_state peek()
{
	if(machine_depth < 1)
		return NULL;
	return machine_stack[machine_depth-1];
}

void complete_transition(CAT_FSM_state state)
{
	if(state == NULL)
		return;

	if(peek() != NULL)
	{
		(peek())(CAT_FSM_SIGNAL_EXIT);
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

	(peek())(CAT_FSM_SIGNAL_ENTER);
}

void CAT_pushdown_transition(CAT_FSM_state state)
{
	next = state;
}

void CAT_pushdown_tick()
{
	if(next != NULL)
	{
		complete_transition(next);
		next = NULL;
	}
	if(peek() != NULL)
		(peek())(CAT_FSM_SIGNAL_TICK);
}

void CAT_pushdown_back()
{
	if(machine_depth > 1)
	{
		pop();
		CAT_pushdown_transition(peek());
	}
}

CAT_FSM_state CAT_pushdown_peek()
{
	return peek();
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
