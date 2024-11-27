#include "cat_machine.h"

#include <stddef.h>
#include "cat_core.h"
#include "cat_math.h"
#include "cat_sprite.h"
#include "cat_input.h"

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

bool CAT_timer_validate(int timer_id)
{
	if(timer_id < 0 || timer_id >= timetable.length)
	{
		CAT_printf("[ERROR] invalid timer ID: %d\n", timer_id);
		return false;
	}
	return true;
}

float CAT_timer_get(int timer_id)
{
	if(!CAT_timer_validate(timer_id))
		return 0.0f;

	return timetable.timers[timer_id];
}

void CAT_timer_set(int timer_id, float t)
{
	if(!CAT_timer_validate(timer_id))
		return;

	timetable.timers[timer_id] = t;
}

void CAT_timer_add(int timer_id, float t)
{
	if(!CAT_timer_validate(timer_id))
		return;

	timetable.timers[timer_id] += t;
}

bool CAT_timer_tick(int timer_id)
{
	if(!CAT_timer_validate(timer_id))
		return false;

	float* timer = &timetable.timers[timer_id];
	if(*timer < timetable.durations[timer_id])
		*timer += CAT_get_delta_time();
	return *timer >= timetable.durations[timer_id];
}

void CAT_timer_reset(int timer_id)
{
	if(!CAT_timer_validate(timer_id))
		return;

	timetable.timers[timer_id] = 0;
}

float CAT_timer_progress(int timer_id)
{
	if(!CAT_timer_validate(timer_id))
		return 0.0f;

	float t = timetable.timers[timer_id] / timetable.durations[timer_id];
	return clampf(t, 0.0f, 1.0f);
}


//////////////////////////////////////////////////////////////////////////
// MACHINE

CAT_machine_state machine = NULL;
CAT_machine_state machine_stack[64];
int machine_depth = 0;

void push_state(CAT_machine_state s)
{
	machine_stack[machine_depth] = s;
	machine_depth += 1;
}

CAT_machine_state pop_state()
{
	machine_depth -= 1;
	return machine_stack[machine_depth];
}

CAT_machine_state peek_state()
{
	return machine_stack[machine_depth-1];
}

void CAT_machine_transition(CAT_machine_state state)
{
	if(state == NULL)
	{
		CAT_printf("[ERROR] machine transition to NULL state\n");
		return;
	}

	if(machine != NULL)
	{
		(machine)(CAT_MACHINE_SIGNAL_EXIT);
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
		push_state(state);

	machine = state;
	(machine)(CAT_MACHINE_SIGNAL_ENTER);	
}

void CAT_machine_tick()
{
	if(machine != NULL)
		(machine)(CAT_MACHINE_SIGNAL_TICK);
}

void CAT_machine_back()
{
	if(machine_depth > 1)
	{
		pop_state();
		CAT_machine_transition(peek_state());
	}
}
