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
	*timer += simulator.delta_time;
	if(*timer >= timetable.durations[timer_id])
	{
		*timer = 0.0f;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
// MACHINE

CAT_machine_state machine;

void CAT_machine_transition(CAT_machine_state state)
{
	if(machine != NULL)
		machine(CAT_MACHINE_SIGNAL_EXIT);
	machine = state;
	if(machine != NULL)
		machine(CAT_MACHINE_SIGNAL_ENTER);
}

void CAT_machine_tick()
{
	if(machine != NULL)
		machine(CAT_MACHINE_SIGNAL_TICK);
}
