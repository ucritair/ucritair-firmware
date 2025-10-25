#include "cat_machine.h"

#include <stddef.h>
#include "cat_core.h"
#include "cat_math.h"
#include "cat_render.h"
#include "cat_input.h"

//////////////////////////////////////////////////////////////////////////
// MACHINE

void FSM_transition(CAT_FSM* machine)
{
	if(machine == NULL)
		return;

	if(machine->state != NULL)
		(*machine->state)(CAT_FSM_SIGNAL_EXIT);
	machine->state = machine->next;
	machine->next = NULL;
	machine->dirty = false;
	if(machine->state != NULL)
		(*machine->state)(CAT_FSM_SIGNAL_ENTER);
}

void CAT_FSM_transition(CAT_FSM* machine, CAT_FSM_state state)
{
	/*if(machine == NULL)
		return;

	if(machine->state != NULL)
		(*machine->state)(CAT_FSM_SIGNAL_EXIT);
	machine->state = state;
	if(machine->state != NULL)
		(*machine->state)(CAT_FSM_SIGNAL_ENTER);*/
	machine->next = state;
	machine->dirty = true;
}

void CAT_FSM_tick(CAT_FSM* machine)
{
	if(machine->dirty)
		FSM_transition(machine);
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

void CAT_latch_set(CAT_latch* s, bool value)
{
	s->current = value;
}

bool CAT_latch_get(CAT_latch* s)
{
	return s->current;
}

bool CAT_latch_flipped(CAT_latch* s)
{
	return s->current != s->last;
}

void CAT_latch_tick(CAT_latch* s)
{
	s->last = s->current;
}

void CAT_timed_latch_raise(CAT_timed_latch* s)
{
	if(!CAT_latch_get(&s->latch))
	{
		CAT_latch_set(&s->latch, true);
		s->timer = 0;
	}
}

bool CAT_timed_latch_get(CAT_timed_latch* s)
{
	return CAT_latch_get(&s->latch);
}

bool CAT_timed_latch_flipped(CAT_timed_latch* s)
{
	return CAT_latch_flipped(&s->latch);
}

void CAT_timed_latch_reset(CAT_timed_latch* s)
{
	CAT_latch_set(&s->latch, false);
	s->timer = 0;
}

float CAT_timed_latch_t(CAT_timed_latch* s)
{
	if(!CAT_latch_get(&s->latch))
		return 0;
	return s->timer / s->timeout;
}

void CAT_timed_latch_tick(CAT_timed_latch* s)
{
	CAT_latch_tick(&s->latch);

	if(!CAT_latch_get(&s->latch))
		return;

	if(s->timer >= s->timeout)
	{
		CAT_latch_set(&s->latch, false);
		s->timer = 0;
	}
	else
	{
		s->timer += CAT_get_delta_time_s();
	}
}
