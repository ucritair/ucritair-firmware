#include <stddef.h>
#include "cat_machine.h"
#include "cat_core.h"

//////////////////////////////////////////////////////////////////////////
// TIMETABLE

CAT_timetable timetable;

void CAT_timetable_init()
{
	timetable.length = 0;
}

int CAT_timer_init(float duration)
{
	if(timetable.length >= CAT_TIMETABLE_MAX_LENGTH)
		return -1;
	int idx = timetable.length;
	timetable.length += 1;

	timetable.timers[idx] = 0.0f;
	timetable.durations[idx] = duration;
	return idx;
}

bool CAT_timer_tick(int timer_id)
{
	float* timer = &timetable.timers[timer_id];
	*timer += CAT_get_delta_time();
	if(*timer >= timetable.durations[timer_id])
	{
		*timer = 0.0f;
		return true;
	}
	return false;
}

void CAT_timer_reset(int timer_id)
{
	timetable.timers[timer_id] = 0;
}


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

void CAT_machine_transition(CAT_machine_state* machine, CAT_machine_state state)
{
	if(*machine != NULL)
		(*machine)(CAT_MACHINE_SIGNAL_EXIT);
	*machine = state;
	if(*machine != NULL)
		(**machine)(CAT_MACHINE_SIGNAL_ENTER);
}

void CAT_machine_tick(CAT_machine_state* machine)
{
	if(*machine != NULL)
		(**machine)(CAT_MACHINE_SIGNAL_TICK);
}