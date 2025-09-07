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




