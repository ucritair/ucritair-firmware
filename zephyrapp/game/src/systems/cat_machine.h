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
} CAT_FSM;
void CAT_FSM_transition(CAT_FSM* machine, CAT_FSM_state state);
void CAT_FSM_tick(CAT_FSM* machine);

void CAT_pushdown_transition(CAT_FSM_state state);
void CAT_pushdown_tick();
void CAT_pushdown_back();
CAT_FSM_state CAT_pushdown_peek();

typedef void (*CAT_render_callback)(void);
void CAT_set_render_callback(CAT_render_callback callback);
CAT_render_callback CAT_get_render_callback();


//////////////////////////////////////////////////////////////////////////
// SWITCH

typedef struct
{
	bool current;
	bool last;
} CAT_switcher;

#define CAT_SWITCHER_INIT(value) (CAT_switcher) {.current = value, .last = value}

void CAT_switch_set(CAT_switcher* s, bool value);
bool CAT_switch_get(CAT_switcher* s);
bool CAT_switch_flipped(CAT_switcher* s);
void CAT_switch_tick(CAT_switcher* s);

typedef struct
{
	CAT_switcher* switcher;
	float timeout;
	float timer;
} CAT_timed_switcher;

#define CAT_TIMED_SWITCHER_INIT(_switcher, _timeout) (CAT_timed_switcher) {.switcher = _switcher, .timeout = _timeout, .timer = 0}

void CAT_timed_switch_raise(CAT_timed_switcher* s);
float CAT_timed_switch_t(CAT_timed_switcher* s);
void CAT_timed_switch_tick(CAT_timed_switcher* s);
void CAT_timed_switch_reset(CAT_timed_switcher* s);




