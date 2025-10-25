#pragma once

#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
// MACHINE

/* PROTOTYPE
void CAT_MS_example(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_FSM_SIGNAL_TICK:
		{
			break;
		}
		case CAT_FSM_SIGNAL_EXIT:
		{
			break;
		}
	}
}
*/

typedef enum
{
	CAT_FSM_SIGNAL_ENTER,
	CAT_FSM_SIGNAL_TICK,
	CAT_FSM_SIGNAL_EXIT
} CAT_FSM_signal;

typedef void (*CAT_FSM_state)(CAT_FSM_signal);

typedef struct
{
	CAT_FSM_state state;
	CAT_FSM_state next;
	bool dirty;
} CAT_FSM;
void CAT_FSM_transition(CAT_FSM* machine, CAT_FSM_state state);
void CAT_FSM_tick(CAT_FSM* machine);

void CAT_pushdown_rebase(CAT_FSM_state state);
void CAT_pushdown_push(CAT_FSM_state state);
void CAT_pushdown_pop();
void CAT_pushdown_tick();
CAT_FSM_state CAT_pushdown_peek();
CAT_FSM_state CAT_pushdown_last();

typedef void (*CAT_render_callback)(void);
void CAT_set_render_callback(CAT_render_callback callback);
CAT_render_callback CAT_get_render_callback();
void CAT_flip_render_callback();


//////////////////////////////////////////////////////////////////////////
// SWITCH

typedef struct
{
	bool current;
	bool last;
} CAT_latch;

#define CAT_LATCH_INIT(value) {.current = value, .last = value}

void CAT_latch_set(CAT_latch* s, bool value);
bool CAT_latch_get(CAT_latch* s);
bool CAT_latch_flipped(CAT_latch* s);
void CAT_latch_tick(CAT_latch* s);

typedef struct
{
	CAT_latch latch;
	float timeout;
	float timer;
} CAT_timed_latch;

#define CAT_TIMED_LATCH_INIT(_timeout) {.latch = CAT_LATCH_INIT(false), .timeout = _timeout, .timer = 0}

void CAT_timed_latch_raise(CAT_timed_latch* s);
bool CAT_timed_latch_get(CAT_timed_latch* s);
bool CAT_timed_latch_flipped(CAT_timed_latch* s);
void CAT_timed_latch_reset(CAT_timed_latch* s);
float CAT_timed_latch_t(CAT_timed_latch* s);
void CAT_timed_latch_tick(CAT_timed_latch* s);
