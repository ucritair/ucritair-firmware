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
	if(machine == NULL)
		return;

	if(machine->state != NULL)
		(*machine->state)(CAT_FSM_SIGNAL_EXIT);
	machine->state = state;
	if(machine->state != NULL)
		(*machine->state)(CAT_FSM_SIGNAL_ENTER);
}

void CAT_FSM_tick(CAT_FSM* machine)
{
	if(machine->state != NULL)
		(*machine->state)(CAT_FSM_SIGNAL_TICK);
}

CAT_FSM_state next = NULL;
CAT_FSM_state pushdown[64];
int pushdown_depth = 0;

static void push(CAT_FSM_state s)
{
	pushdown[pushdown_depth] = s;
	pushdown_depth += 1;
}

static CAT_FSM_state pop()
{
	pushdown_depth -= 1;
	return pushdown[pushdown_depth];
}

static CAT_FSM_state peek()
{
	if(pushdown_depth < 1)
		return NULL;
	return pushdown[pushdown_depth-1];
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
	for(int i = 0; i < pushdown_depth; i++)
	{
		if(pushdown[i] == state)
		{
			pushdown_depth = i+1;
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
	if(pushdown_depth > 1)
	{
		pop();
		CAT_pushdown_transition(peek());
	}
}

CAT_FSM_state CAT_pushdown_peek()
{
	return peek();
}

static CAT_render_callback render_callback[2] =
{
	NULL, NULL
};

void CAT_set_render_callback(CAT_render_callback callback)
{
	render_callback[1] = callback;
}

CAT_render_callback CAT_get_render_callback()
{
	return render_callback[0];
}

void CAT_flip_render_callback()
{
	if(render_callback[1] != NULL)
		render_callback[0] = render_callback[1];
	render_callback[1] = NULL;
}


//////////////////////////////////////////////////////////////////////////
// SWITCH

void CAT_switch_set(CAT_switcher* s, bool value)
{
	s->current = value;
}

bool CAT_switch_get(CAT_switcher* s)
{
	return s->current;
}

bool CAT_switch_flipped(CAT_switcher* s)
{
	return s->current != s->last;
}

void CAT_switch_tick(CAT_switcher* s)
{
	s->last = s->current;
}

void CAT_timed_switch_raise(CAT_timed_switcher* s)
{
	if(!CAT_switch_get(&s->switcher))
	{
		CAT_switch_set(&s->switcher, true);
		s->timer = 0;
	}
}

bool CAT_timed_switch_get(CAT_timed_switcher* s)
{
	return CAT_switch_get(&s->switcher);
}

bool CAT_timed_switch_flipped(CAT_timed_switcher* s)
{
	return CAT_switch_flipped(&s->switcher);
}

float CAT_timed_switch_t(CAT_timed_switcher* s)
{
	if(!CAT_switch_get(&s->switcher))
		return 0;
	return s->timer / s->timeout;
}

void CAT_timed_switch_tick(CAT_timed_switcher* s)
{
	CAT_switch_tick(&s->switcher);

	if(!CAT_switch_get(&s->switcher))
		return;

	if(s->timer >= s->timeout)
	{
		CAT_switch_set(&s->switcher, false);
		s->timer = 0;
	}
	else
	{
		s->timer += CAT_get_delta_time_s();
	}
}

void CAT_timed_switch_reset(CAT_timed_switcher* s)
{
	CAT_switch_set(&s->switcher, false);
	s->timer = 0;
}
