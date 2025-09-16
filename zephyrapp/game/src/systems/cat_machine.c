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

static CAT_FSM_state pushdown_memory[64];
static int pushdown_depth = 0;
#define MAX_PUSHDOWN_DEPTH (sizeof(pushdown_memory)/sizeof(pushdown_memory[0]))

static CAT_FSM_state transition_target;
static enum {NONE, REBASE, PUSH, POP} transition_mode;

static void push(CAT_FSM_state s)
{
	pushdown_memory[pushdown_depth] = s;
	pushdown_depth += 1;
}

static CAT_FSM_state pop()
{
	pushdown_depth -= 1;
	return pushdown_memory[pushdown_depth];
}

static CAT_FSM_state peek()
{
	if(pushdown_depth == 0)
		return NULL;
	return pushdown_memory[pushdown_depth-1];
}

void pushdown_transition()
{
	if(transition_mode == POP)
	{
		if(pushdown_depth <= 0)
			return;

		if(peek() != NULL)
			peek()(CAT_FSM_SIGNAL_EXIT);
		pop();
		if(peek() != NULL)
			peek()(CAT_FSM_SIGNAL_ENTER);
	}
	else
	{
		if(transition_target == NULL)
			return;
		if(pushdown_depth == MAX_PUSHDOWN_DEPTH)
			transition_mode = REBASE;
		
		if(peek() != NULL)
			peek()(CAT_FSM_SIGNAL_EXIT);
		if(transition_mode == REBASE)
			pushdown_depth = 0;
		push(transition_target);
		if(peek() != NULL)
			peek()(CAT_FSM_SIGNAL_ENTER);
	}

	transition_mode = NONE;
}

void CAT_pushdown_rebase(CAT_FSM_state state)
{
	transition_target = state;
	transition_mode = REBASE;
}

void CAT_pushdown_push(CAT_FSM_state state)
{
	transition_target = state;
	transition_mode = PUSH;
}

void CAT_pushdown_pop()
{
	transition_target = NULL;
	transition_mode = POP;
}

void CAT_pushdown_tick()
{
	if(transition_mode != NONE)
		pushdown_transition();
	CAT_printf("%d\n", pushdown_depth);
	if(peek() != NULL)
		peek()(CAT_FSM_SIGNAL_TICK);
}

CAT_FSM_state CAT_pushdown_peek()
{
	return peek();
}

CAT_FSM_state CAT_pushdown_last()
{
	if(pushdown_depth > 1)
		return pushdown_memory[pushdown_depth-2];
	return NULL;
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
