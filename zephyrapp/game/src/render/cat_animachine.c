#include "cat_render.h"

//////////////////////////////////////////////////////////////////////////
// ANIMATION MACHINE

void CAT_animachine_init(CAT_animachine_state* state, const CAT_sprite* enai, const CAT_sprite* tiai, const CAT_sprite* exai)
{
	state->signal = ENTER;
	state->enter_anim_id = enai;
	state->tick_anim_id = tiai;
	state->exit_anim_id = exai;
	if(enai != NULL)
		state->last = enai;
	else if(tiai != NULL)
		state->last = tiai;
	else if(exai != NULL)
		state->last = exai;
	else
		state->last = NULL;
	state->next = NULL;
}

void CAT_animachine_transition(CAT_animachine_state** spp, CAT_animachine_state* next)
{
	if(next == NULL)
	{
		*spp = NULL;
		return;
	}

	CAT_animachine_state* sp = *spp;
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

const CAT_sprite* CAT_animachine_tick(CAT_animachine_state** spp)
{
	if(*spp == NULL)
		return NULL;
	CAT_animachine_state* sp = *spp;

	switch(sp->signal)
	{
		case ENTER:
		{
			if(CAT_anim_finished(sp->enter_anim_id))
			{
				sp->signal = TICK;
				break;
			}
			sp->last = sp->enter_anim_id;
			return sp->enter_anim_id;
			
		}
		case TICK:
		{
			if(sp->tick_anim_id == NULL)
			{
				sp->signal = EXIT;
				break;
			}
			sp->last = sp->tick_anim_id;
			return sp->tick_anim_id;
		}
		case EXIT:
		{
			if(CAT_anim_finished(sp->exit_anim_id))
			{
				sp->signal = DONE;
				break;
			}
			sp->last = sp->exit_anim_id;
			return sp->exit_anim_id;
		}
		default:
		{
			if(sp->next != NULL)
			{
				CAT_animachine_state* next = sp->next;
				sp->next = NULL;

				next->signal = ENTER;
				CAT_anim_reset(next->enter_anim_id);
				CAT_anim_reset(next->tick_anim_id);
				CAT_anim_reset(next->exit_anim_id);
				*spp = next;
			}
			break;
		}
	}

	return sp->last;
}

void CAT_animachine_kill(CAT_animachine_state** spp)
{
	if(*spp == NULL)
		return;

	if((*spp)->signal != DONE)
		(*spp)->signal = DONE;
}

bool CAT_animachine_is_in(CAT_animachine_state** spp, CAT_animachine_state* state)
{
	if(*spp == NULL)
		return false;
	return (*spp) == state;
}

bool CAT_animachine_is_ticking(CAT_animachine_state** spp)
{
	if(*spp == NULL)
		return false;
	return (*spp)->signal == TICK;
}

bool CAT_animachine_is_done(CAT_animachine_state** spp)
{
	if(*spp == NULL)
		return false;
	return (*spp)->signal == DONE;
}


//////////////////////////////////////////////////////////////////////////
// DECLARATIONS AND DEFINITIONS

// MACHINES
CAT_animachine_state* pet_asm;

CAT_animachine_state AS_idle;
CAT_animachine_state AS_walk;
CAT_animachine_state AS_crit;

CAT_animachine_state AS_adjust_in;
CAT_animachine_state AS_approach;
CAT_animachine_state AS_adjust_out;

CAT_animachine_state AS_eat;
CAT_animachine_state AS_study;
CAT_animachine_state AS_play;

CAT_animachine_state AS_vig_up;
CAT_animachine_state AS_foc_up;
CAT_animachine_state AS_spi_up;

CAT_animachine_state* react_asm;
CAT_animachine_state AS_react;

void CAT_sprite_mass_define()
{
	// PET STATES
	CAT_anim_toggle_loop(&pet_high_vig_in_sprite, false);
	CAT_anim_toggle_reverse(&pet_high_vig_out_sprite, true);
	CAT_anim_toggle_loop(&pet_high_vig_out_sprite, false);

	CAT_anim_toggle_loop(&pet_crit_vig_in_sprite, false);
	CAT_anim_toggle_reverse(&pet_crit_vig_out_sprite, true);
	CAT_anim_toggle_loop(&pet_crit_vig_out_sprite, false);

	CAT_anim_toggle_loop(&pet_crit_foc_in_sprite, false);
	CAT_anim_toggle_reverse(&pet_crit_foc_out_sprite, true);
	CAT_anim_toggle_loop(&pet_crit_foc_out_sprite, false);

	CAT_anim_toggle_loop(&pet_crit_spi_in_sprite, false);
	CAT_anim_toggle_reverse(&pet_crit_spi_out_sprite, true);
	CAT_anim_toggle_loop(&pet_crit_spi_out_sprite, false);

	// PET ACTIONS
	CAT_anim_toggle_loop(&pet_eat_in_sprite, false);
	CAT_anim_toggle_reverse(&pet_eat_out_sprite, true);
	CAT_anim_toggle_loop(&pet_eat_out_sprite, false);

	CAT_anim_toggle_loop(&pet_study_in_sprite, false);
	CAT_anim_toggle_reverse(&pet_study_out_sprite, true);
	CAT_anim_toggle_loop(&pet_study_out_sprite, false);


	// MACHINES
	CAT_animachine_init(&AS_idle, NULL, &pet_idle_sprite, NULL);
	CAT_animachine_init(&AS_walk, NULL, &pet_walk_sprite, NULL);
	CAT_animachine_init(&AS_crit, &pet_crit_vig_in_sprite, &pet_crit_vig_sprite, &pet_crit_vig_out_sprite);

	CAT_animachine_init(&AS_approach, NULL, &pet_walk_sprite, NULL);

	CAT_animachine_init(&AS_eat, &pet_eat_in_sprite, &pet_eat_sprite, &pet_eat_out_sprite);
	CAT_animachine_init(&AS_study, &pet_study_in_sprite, &pet_study_sprite, &pet_study_out_sprite);
	CAT_animachine_init(&AS_play, NULL, &pet_play_a_sprite, NULL);

	CAT_animachine_init(&AS_vig_up, NULL, NULL, &pet_vig_up_sprite);
	CAT_animachine_init(&AS_foc_up, NULL, NULL, &pet_foc_up_sprite);
	CAT_animachine_init(&AS_spi_up, NULL, NULL, &pet_spi_up_sprite);

	CAT_animachine_init(&AS_react, NULL, &mood_good_sprite, NULL);
}
