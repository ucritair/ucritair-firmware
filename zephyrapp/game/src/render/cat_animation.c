#include "cat_render.h"

#include "sprite_assets.h"

//////////////////////////////////////////////////////////////////////////
// ANIMATOR

static int frame_indices[CAT_SPRITE_LIST_LENGTH];
static int frame_counter = 0;
static bool cycle_complete[CAT_SPRITE_LIST_LENGTH];
#define CAT_ANIMATOR_FRAME_PERIOD 1

void CAT_animator_init()
{
	for(int i = 0; i < CAT_SPRITE_LIST_LENGTH; i++)
	{
		frame_indices[i] = 0;
		cycle_complete[i] = false;
	}
}

void CAT_animator_tick()
{
	for(int i = 0; i < CAT_SPRITE_LIST_LENGTH; i++)
		cycle_complete[i] = false;

	if(frame_counter == CAT_ANIMATOR_FRAME_PERIOD)
	{
		frame_counter = 0;
		for(int i = 0; i < CAT_SPRITE_LIST_LENGTH; i++)
		{
			const CAT_sprite* sprite = sprite_list[i];
			if(frame_indices[i] < sprite->frame_count-1)
			{
				frame_indices[i] += 1;
			}
			else
			{
				if(sprite->loop)
					frame_indices[i] = 0;
				cycle_complete[i] = true;
			}
		}
	}
	else
	{
		frame_counter += 1;
	}
}

int CAT_animator_get_frame(const CAT_sprite* sprite)
{
	if(sprite == NULL)
		return -1;
	int idx = frame_indices[sprite->id];
	if(sprite->reverse)
		idx = sprite->frame_count - idx - 1;
	return idx;
}

bool animator_is_finished(const CAT_sprite* sprite)
{
	if(sprite == NULL)
		return true;
	return cycle_complete[sprite->id];
}

void animator_reset(const CAT_sprite* sprite)
{
	if(sprite == NULL)
		return;
	frame_indices[sprite->id] = 0;
	cycle_complete[sprite->id] = false;
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
	if(next == machine->state)
		return;
	machine->next = next;
}

void CAT_anim_tick(CAT_anim_machine* machine)
{
	if(machine == NULL)
		return;
	if(machine->state == NULL)
	{
		if(machine->next != NULL)
		{
			machine->state = machine->next;
			machine->next = NULL;
			animator_reset(machine->state->enter_sprite);
			machine->signal = ENTER;
		}
		else
			return;
	}
	
	if(machine->signal == ENTER)
	{
		if(animator_is_finished(machine->state->enter_sprite))
		{
			animator_reset(machine->state->tick_sprite);
			machine->signal = TICK;
		}
	}
	if(machine->signal == TICK)
	{
		if(machine->state->tick_sprite == NULL  || machine->next != NULL)
		{
			animator_reset(machine->state->exit_sprite);
			machine->signal = EXIT;
		}
	}
	if(machine->signal == EXIT)
	{
		if(animator_is_finished(machine->state->exit_sprite))
		{
			machine->state = NULL;
			CAT_anim_tick(machine);
		}
	}
}

const CAT_sprite* CAT_anim_read(CAT_anim_machine* machine)
{
	if(machine == NULL)
		return NULL;
	if(machine->state == NULL)
		return NULL;

	switch(machine->signal)
	{
		case ENTER:
			return machine->state->enter_sprite;
			break;
		case TICK:
			return machine->state->tick_sprite;
			break;
		case EXIT:
			return machine->state->exit_sprite;
			break;
		default:
			return NULL;
	}
}

void CAT_anim_kill(CAT_anim_machine* machine)
{
	machine->state = NULL;
}

bool CAT_anim_is_dead(CAT_anim_machine* machine)
{
	return machine->state == NULL;
}

bool CAT_anim_is_in(CAT_anim_machine* machine, CAT_anim_state* state)
{
	return machine->state == state;
}

bool CAT_anim_is_ticking(CAT_anim_machine* machine)
{
	return machine->signal == TICK;
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
