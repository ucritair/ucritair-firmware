#include "sprite_assets.h"

//////////////////////////////////////////////////////////////////////////
// ANIMATOR

static int frame_indices[CAT_SPRITE_LIST_LENGTH];
static int frame_counter = 0;
#define CAT_ANIMATOR_FRAME_PERIOD 2

void CAT_animator_init()
{
	for(int i = 0; i < CAT_SPRITE_LIST_LENGTH; i++)
	{
		frame_indices[i] = 0;
	}
}

void CAT_animator_tick()
{
	if(frame_counter == CAT_ANIMATOR_FRAME_PERIOD)
	{
		frame_counter = 0;
		for(int i = 0; i < CAT_SPRITE_LIST_LENGTH; i++)
		{
			const CAT_sprite* sprite = sprite_list[i];
			frame_indices[i] += 1;
			if(frame_indices[i] < sprite->frame_count-1)
				frame_indices[i] += 1;
			else if(sprite->loop)
				frame_indices[i] = 0;
		}
	}
	else
	{
		frame_counter += 1;
	}
}

int CAT_animator_get_frame(const CAT_sprite* sprite)
{
	int idx = frame_indices[sprite->id];
	if(sprite->reverse)
		idx = sprite->frame_count - idx - 1;
	return idx;
}

void animator_finish(const CAT_sprite* sprite)
{
	frame_indices[sprite->id] = sprite->frame_count;
}

bool animator_is_finished(const CAT_sprite* sprite)
{
	return frame_indices[sprite->id] >= sprite->frame_count-1;
}

void animator_reset(const CAT_sprite* sprite)
{
	frame_indices[sprite->id] = 0;
}


//////////////////////////////////////////////////////////////////////////
// ANIM GRAPH

void CAT_anim_init(CAT_anim_state* state, const CAT_sprite* enter, const CAT_sprite* tick, const CAT_sprite* exit)
{
	state->enter_sprite = enter;
	state->tick_sprite = tick;
	state->exit_sprite = exit;
}

void CAT_anim_transition(CAT_anim_machine* machine, CAT_anim_state* next)
{
	machine->next = next;
}

const CAT_sprite* CAT_anim_tick(CAT_anim_machine* machine)
{
	if(CAT_is_first_render_cycle())
	{
		// Finalize transition
		if(machine->next == NULL)
			machine->signal = EXIT;

		// Fast-forward
		if(machine->signal == ENTER)
		{
			if(machine->state->enter_sprite == NULL)
				machine->signal = TICK;
		}
		if(machine->signal == TICK)
		{
			if(machine->state->tick_sprite == NULL)
				machine->signal = EXIT;
		}
		if(machine->signal == EXIT)
		{
			if(machine->state->exit_sprite == NULL)
			{
				machine->state = machine->next;
				machine->signal = ENTER;
			}
		}
			
		// Evaluate at final destination
		switch(machine->signal)
		{
			case(ENTER):
			{
				machine->sprite = machine->state->enter_sprite;
				if(animator_is_finished(machine->state->enter_sprite))
					machine->signal = TICK;
				break;
			}
			case(TICK):
			{
				machine->sprite = machine->state->tick_sprite;
				if(machine->next != NULL)
					machine->signal = EXIT;
				break;
			}
			case(EXIT):
			{
				machine->sprite = machine->state->exit_sprite;
				if(animator_is_finished(machine->state->exit_sprite))
				{	
					machine->state = machine->next;
					machine->signal = ENTER;
				}
				break;
			}
		}
	}

	return machine->sprite;
}

void CAT_anim_kill(CAT_anim_machine* machine)
{
	machine->signal = EXIT;
}

bool CAT_anim_is_in(CAT_anim_machine* machine, CAT_anim_state* state)
{
	return (*machine) == state;
}

bool CAT_anim_is_ticking(CAT_anim_machine* machine)
{
	return (*machine)->signal == TICK;
}

bool CAT_anim_is_done(CAT_anim_machine* machine)
{
	return (*machine)->signal == EXIT && animator_is_finished((*machine)->exit_sprite);
}


//////////////////////////////////////////////////////////////////////////
// DEFINITIONS

CAT_anim_state AS_idle =
{
	.enter_sprite = NULL,
	.tick_sprite = &pet_idle_sprite,
	.exit_sprite = NULL
};

CAT_anim_state AS_walk =
{
	.enter_sprite = NULL,
	.tick_sprite = &pet_walk_sprite,
	.exit_sprite = NULL
};

CAT_anim_state AS_crit =
{
	.enter_sprite = &pet_crit_vig_in_sprite,
	.tick_sprite = &pet_crit_vig_sprite,
	.exit_sprite = &pet_crit_vig_out_sprite
};

CAT_anim_state AS_eat =
{
	.enter_sprite = &pet_eat_in_sprite,
	.tick_sprite = &pet_eat_sprite,
	.exit_sprite = &pet_eat_out_sprite
};

CAT_anim_state AS_study =
{
	.enter_sprite = &pet_study_in_sprite,
	.tick_sprite = &pet_study_sprite,
	.exit_sprite = &pet_study_out_sprite
};

CAT_anim_state AS_play =
{
	.enter_sprite = NULL,
	.tick_sprite = &pet_play_a_sprite,
	.exit_sprite = NULL
};

CAT_anim_state AS_vig_up =
{
	.enter_sprite = &pet_vig_up_sprite,
	.tick_sprite = NULL,
	.exit_sprite = NULL
};

CAT_anim_state AS_foc_up =
{
	.enter_sprite = &pet_foc_up_sprite,
	.tick_sprite = NULL,
	.exit_sprite = NULL
};

CAT_anim_state AS_spi_up =
{
	.enter_sprite = &pet_spi_up_sprite,
	.tick_sprite = NULL,
	.exit_sprite = NULL
};

CAT_anim_state AS_react =
{
	.enter_sprite = NULL,
	.tick_sprite = &mood_good_sprite,
	.exit_sprite = NULL
};
