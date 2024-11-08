#include "cat_machine.h"
#include <stddef.h>
#include "cat_core.h"
#include "cat_math.h"
#include "cat_sprite.h"

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
	return *timer >= timetable.durations[timer_id];
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


//////////////////////////////////////////////////////////////////////////
// ANIMACHINE

void CAT_AM_init(CAT_AM_state* state, int enai, int tiai, int exai)
{
	state->signal = ENTER;
	state->enter_anim_id = enai;
	state->tick_anim_id = tiai;
	state->exit_anim_id = exai;
	state->next = NULL;
}

void CAT_AM_transition(CAT_AM_state** spp, CAT_AM_state* next)
{
	if(next == NULL)
	{
		*spp = NULL;
		return;
	}

	CAT_AM_state* sp = *spp;
	if(sp != NULL)
	{
		if(sp->signal != DONE)
			sp->signal = EXIT;
		sp->next = next;
	}
	else
	{
		next->signal = ENTER;
		CAT_anim_reset(next->enter_anim_id);
		CAT_anim_reset(next->tick_anim_id);
		CAT_anim_reset(next->exit_anim_id);
		*spp = next;
	}
}

void CAT_AM_kill(CAT_AM_state** spp)
{
	if(*spp == NULL)
		return;
	if((*spp)->signal != DONE)
		(*spp)->signal = EXIT;
}

bool CAT_AM_is_in(CAT_AM_state** spp, CAT_AM_state* state)
{
	if(*spp == NULL)
		return false;
	return (*spp) == state;
}

bool CAT_AM_is_entering(CAT_AM_state** spp)
{
	if(*spp == NULL)
		return false;
	return (*spp)->signal == ENTER;
}

bool CAT_AM_is_ticking(CAT_AM_state** spp)
{
	if(*spp == NULL)
		return false;
	return (*spp)->signal == TICK;
}

bool CAT_AM_is_exiting(CAT_AM_state** spp)
{
	if(*spp == NULL)
		return false;
	return (*spp)->signal == EXIT;
}

bool CAT_AM_is_done(CAT_AM_state** spp)
{
	if(*spp == NULL)
		return false;
	return (*spp)->signal == DONE;
}

int CAT_AM_tick(CAT_AM_state** spp)
{
	if(*spp == NULL)
		return -1;

	CAT_AM_state* sp = *spp;
	if(sp->signal == ENTER)
	{
		if(!CAT_anim_finished(sp->enter_anim_id))
			return sp->enter_anim_id;
		sp->signal = TICK;
	}
	if(sp->signal == TICK)
	{
		if(sp->tick_anim_id != -1)
			return sp->tick_anim_id;
		sp->signal = EXIT;
	}
	if(sp->signal == EXIT)
	{
		if(!CAT_anim_finished(sp->exit_anim_id))
			return sp->exit_anim_id;
		sp->signal = DONE;
	}

	if(sp->next != NULL)
	{
		CAT_AM_state* next = sp->next;
		sp->next = NULL;

		next->signal = ENTER;
		CAT_anim_reset(next->enter_anim_id);
		CAT_anim_reset(next->tick_anim_id);
		CAT_anim_reset(next->exit_anim_id);
		*spp = next;
	}

	return sp->exit_anim_id != -1 ? sp->exit_anim_id : sp->tick_anim_id;
}
