#include "cat_machine.h"

#include <stddef.h>
#include "cat_core.h"
#include "cat_math.h"
#include "cat_render.h"
#include "cat_input.h"

//////////////////////////////////////////////////////////////////////////
// TIMETABLE

CAT_timetable timetable;

void CAT_timetable_init()
{
	for(int i = 0; i < CAT_TIMETABLE_MAX_LENGTH; i++)
	{
		timetable.active[i] = false;
		timetable.duration[i] = 0.0f;
		timetable.timer[i] = 0.0f;
	}
}

int CAT_timer_init(float duration)
{
	for(int i = 0; i < CAT_TIMETABLE_MAX_LENGTH; i++)
	{
		if(!timetable.active[i])
		{
			timetable.active[i] = true;
			timetable.duration[i] = duration;
			timetable.timer[i] = 0.0f;
			return i;
		}
	}
	
	CAT_printf("[WARNING] attempted add to full timetable!\n");
	return -1;
}

void CAT_timer_reinit(int* timer_id, float duration)
{
	int idx = *timer_id;
	if(idx == -1 || idx >= CAT_TIMETABLE_MAX_LENGTH || !timetable.active[idx])
	{
		*timer_id = CAT_timer_init(duration);
	}
	else
	{
		timetable.duration[idx] = duration;
		timetable.timer[idx] = 0.0f;
	}
}

void CAT_timer_delete(int timer_id)
{
	timetable.active[timer_id] = false;
}

bool CAT_timer_validate(int timer_id)
{
	if(timer_id < 0 || timer_id >= CAT_TIMETABLE_MAX_LENGTH)
	{
		CAT_printf("[ERROR] invalid timer ID: %d\n", timer_id);
		return false;
	}
	if(!timetable.active[timer_id])
	{
		CAT_printf("[ERROR] inactive timer ID: %d\n", timer_id);
		return false;
	}
	return true;
}

float CAT_timer_get(int timer_id)
{
	if(!CAT_timer_validate(timer_id))
		return 0.0f;

	return timetable.timer[timer_id];
}

void CAT_timer_set(int timer_id, float t)
{
	if(!CAT_timer_validate(timer_id))
		return;

	timetable.timer[timer_id] = t;
}

void CAT_timer_add(int timer_id, float t)
{
	if(!CAT_timer_validate(timer_id))
		return;

	timetable.timer[timer_id] += t;
}

bool CAT_timer_tick(int timer_id)
{
	if(!CAT_timer_validate(timer_id))
		return false;

	float* timer = &timetable.timer[timer_id];
	if(*timer < timetable.duration[timer_id])
		*timer += CAT_get_delta_time_s();
	return *timer >= timetable.duration[timer_id];
}

void CAT_timer_reset(int timer_id)
{
	if(!CAT_timer_validate(timer_id))
		return;

	timetable.timer[timer_id] = 0.0f;
}

bool CAT_timer_done(int timer_id)
{
	if(!CAT_timer_validate(timer_id))
		return false;
	return timetable.timer[timer_id] >= timetable.duration[timer_id];
}

float CAT_timer_progress(int timer_id)
{
	if(!CAT_timer_validate(timer_id))
		return 0.0f;

	float t = timetable.timer[timer_id] / timetable.duration[timer_id];
	return clamp(t, 0.0f, 1.0f);
}

bool CAT_pulse(float period)
{
	int T = period * 1000.0f;
	int t = CAT_get_uptime_ms();
	return t % (2*T) < T;
}


//////////////////////////////////////////////////////////////////////////
// MACHINE

CAT_machine_state current = NULL;
CAT_machine_state next = NULL;
CAT_machine_state machine_stack[64];
int machine_depth = 0;

static void push(CAT_machine_state s)
{
	machine_stack[machine_depth] = s;
	machine_depth += 1;
}

static CAT_machine_state pop()
{
	machine_depth -= 1;
	return machine_stack[machine_depth];
}

static CAT_machine_state peek()
{
	return machine_stack[machine_depth-1];
}

void complete_transition(CAT_machine_state state)
{
	if(state == NULL)
	{
		CAT_printf("[ERROR] machine transition to NULL state\n");
		return;
	}

	if(current != NULL)
	{
		(current)(CAT_MACHINE_SIGNAL_EXIT);
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
		push(state);

	current = state;
	(current)(CAT_MACHINE_SIGNAL_ENTER);
}

void CAT_machine_transition(CAT_machine_state state)
{
	next = state;
}

void CAT_machine_tick()
{
	if(next != NULL)
	{
		complete_transition(next);
		next = NULL;
	}
	if(current != NULL)
		(current)(CAT_MACHINE_SIGNAL_TICK);
}

void CAT_machine_back()
{
	if(machine_depth > 1)
	{
		pop();
		CAT_machine_transition(peek());
	}
}

CAT_machine_state CAT_get_machine_state()
{
	return current;
}

CAT_render_callback render_callback;

void CAT_set_render_callback(CAT_render_callback callback)
{
	render_callback = callback;
}

CAT_render_callback CAT_get_render_callback()
{
	return render_callback;
}
