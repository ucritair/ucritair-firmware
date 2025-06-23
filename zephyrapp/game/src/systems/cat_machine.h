#pragma once

#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
// MACHINE

/* PROTOTYPE
void CAT_MS_example(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}
*/

typedef enum CAT_machine_signal
{
	CAT_MACHINE_SIGNAL_ENTER,
	CAT_MACHINE_SIGNAL_TICK,
	CAT_MACHINE_SIGNAL_EXIT
} CAT_machine_signal;

typedef void (*CAT_machine_state)(CAT_machine_signal);
typedef void (*CAT_render_callback)(void);

void CAT_machine_transition(CAT_machine_state state);
void CAT_machine_tick();
void CAT_machine_back();
CAT_machine_state CAT_get_machine_state();

void CAT_set_render_callback(CAT_render_callback callback);
CAT_render_callback CAT_get_render_callback();


