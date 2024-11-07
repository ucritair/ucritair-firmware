#include <stddef.h>
#include "cat_machine.h"
#include "cat_core.h"
#include "cat_math.h"

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

void CAT_ASM_init(CAT_ASM_state* state, int enai, int tiai, int exai)
{
	state->signal = ENTER;
	state->enter_anim_id = enai;
	state->tick_anim_id = tiai;
	state->exit_anim_id = exai;
	state->next = NULL;
}

void CAT_ASM_transition(CAT_ASM_state** spp, CAT_ASM_state* next)
{
	CAT_ASM_state* sp = *spp;
	if(sp != NULL && sp->signal != DONE)
	{
		sp->signal = EXIT;
		sp->next = next;
	}
	else if(next != NULL)
	{
		next->signal = ENTER;
		*spp = next;
	}
	else
	{
		*spp = NULL;
	}
}

void CAT_ASM_kill(CAT_ASM_state** spp)
{
	if(*spp == NULL)
		return;
	if((*spp)->signal != DONE)
		(*spp)->signal = EXIT;
}

bool CAT_ASM_is_in(CAT_ASM_state** spp, CAT_ASM_state* state)
{
	if(*spp == NULL)
		return false;
	return (*spp) == state;
}

bool CAT_ASM_is_entering(CAT_ASM_state** spp)
{
	if(*spp == NULL)
		return false;
	return (*spp)->signal == ENTER;
}

bool CAT_ASM_is_ticking(CAT_ASM_state** spp)
{
	if(*spp == NULL)
		return false;
	return (*spp)->signal == TICK;
}

bool CAT_ASM_is_exiting(CAT_ASM_state** spp)
{
	if(*spp == NULL)
		return false;
	return (*spp)->signal == EXIT;
}

bool CAT_ASM_is_done(CAT_ASM_state** spp)
{
	if(*spp == NULL)
		return false;
	return (*spp)->signal == DONE;
}

int CAT_ASM_tick(CAT_ASM_state** spp)
{
	if(*spp == NULL)
		return -1;

	CAT_ASM_state* sp = *spp;
	if(sp->signal == ENTER)
	{
		if(sp->enter_anim_id != -1)
		{
			if(CAT_anim_finished(sp->enter_anim_id))
				CAT_anim_reset(sp->enter_anim_id);
			else
				return sp->enter_anim_id;
		}
		sp->signal = TICK;
		return sp->tick_anim_id;
	}
	else if(sp->signal == TICK)
	{
		return sp->tick_anim_id;
	}
	if(sp->signal == EXIT && sp->exit_anim_id != -1)
	{
		if(CAT_anim_finished(sp->exit_anim_id))
			CAT_anim_reset(sp->exit_anim_id);
		else
			return sp->exit_anim_id;	
	}

	if(sp->next != NULL)
	{
		CAT_ASM_state* next = sp->next;
		sp->next = NULL;

		next->signal = ENTER;
		*spp = next;
	}

	sp->signal = DONE;
	return sp->exit_anim_id != -1 ? sp->exit_anim_id : sp->tick_anim_id;
}
