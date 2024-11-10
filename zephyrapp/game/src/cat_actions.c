#include "cat_actions.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_input.h"
#include "cat_bag.h"
#include "cat_pet.h"
#include "cat_sprite.h"
#include <stdio.h>
#include <stddef.h>

CAT_action_state action_state = 
{
	.action_MS = NULL,
	.action_AS = NULL,
	.stat_up_AS = NULL,
	.item_id = -1,
	.confirmed = false,
	.complete = false
};

void CAT_action_state_clear()
{
	action_state.item_id = -1;
	action_state.confirmed = false;
	action_state.complete = false;
}

void CAT_action_tick()
{
	if(action_state.item_id == -1)
	{
		bag_state.objective = action_state.action_MS;
		CAT_machine_transition(&machine, CAT_MS_bag);
	}
	else if(!action_state.confirmed)
	{
		CAT_room_move_cursor();

		if(CAT_input_pressed(CAT_BUTTON_A))
		{
			CAT_ivec2 c_world = CAT_ivec2_mul(room.cursor, 16);
			int x_off = c_world.x > pet.pos.x ? -16 : 32;
			action_state.location = (CAT_vec2) {c_world.x + x_off, c_world.y + 16};
			action_state.confirmed = true;
			CAT_AM_transition(&pet_asm, &AS_adjust_in);
		}
	}
	else
	{
		if(CAT_AM_is_in(&pet_asm, &AS_adjust_in))
			CAT_AM_transition(&pet_asm, &AS_walk_action);
		if(CAT_AM_is_in(&pet_asm, &AS_walk_action) && CAT_AM_is_ticking(&pet_asm))
		{
			if(CAT_pet_seek(action_state.location))
			{
				pet.left = (room.cursor.x * 16) > pet.pos.x;
				CAT_AM_transition(&pet_asm, action_state.action_AS);
			}
		}
		if(CAT_AM_is_in(&pet_asm, action_state.action_AS) && CAT_AM_is_ticking(&pet_asm))
		{
			if(CAT_timer_tick(pet.action_timer_id))
			{
				CAT_item* item = CAT_item_get(action_state.item_id);
				pet.vigour += item->data.tool_data.dv;
				pet.focus += item->data.tool_data.df;
				pet.spirit += item->data.tool_data.ds;
				CAT_bag_remove(action_state.item_id);
				action_state.complete = true;
				CAT_AM_kill(&pet_asm);
				CAT_AM_transition(&pet_asm, action_state.stat_up_AS);
				CAT_timer_reset(pet.action_timer_id);
			}
		}
		if(CAT_AM_is_in(&pet_asm, action_state.stat_up_AS))
			CAT_AM_transition(&pet_asm, &AS_adjust_out);
		if(CAT_AM_is_in(&pet_asm, &AS_adjust_out))
			CAT_machine_transition(&machine, CAT_MS_room);
	}

	if(CAT_input_pressed(CAT_BUTTON_B))
		CAT_machine_transition(&machine, CAT_MS_room);
}

void CAT_MS_feed(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			action_state.action_MS = CAT_MS_feed;
			action_state.action_AS = &AS_eat;
			action_state.stat_up_AS = &AS_vig_up;
			CAT_AM_transition(&pet_asm, &AS_idle);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_action_tick();
			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			CAT_action_state_clear();
			break;
		}
	}
}

void CAT_MS_study(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			action_state.action_MS = CAT_MS_study;
			action_state.action_AS = &AS_study;
			action_state.stat_up_AS = &AS_foc_up;
			CAT_AM_transition(&pet_asm, &AS_idle);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_action_tick();
			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			CAT_action_state_clear();
			break;
		}
	}
}

void CAT_MS_play(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			action_state.action_MS = CAT_MS_play;
			action_state.action_AS = &AS_play;
			action_state.stat_up_AS = &AS_spi_up;
			CAT_AM_transition(&pet_asm, &AS_idle);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_action_tick();
			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			CAT_action_state_clear();
			break;
		}
	}
}

void CAT_render_action(int cycle)
{
	if(cycle == 0)
	{
		CAT_ivec2 spot = CAT_ivec2_mul(room.cursor, 16);
		if(action_state.item_id != -1)
		{
			CAT_item* item = CAT_item_get(action_state.item_id);	
			if(!action_state.confirmed)
			{
				CAT_draw_queue_add(item->data.tool_data.cursor_sprite_id, 0, 2, spot.x, spot.y, CAT_DRAW_MODE_DEFAULT);
			}			
			else if(!action_state.complete)
			{
				CAT_draw_queue_animate(item->sprite_id, 2, spot.x, spot.y, CAT_DRAW_MODE_DEFAULT);
			}
		}
		else
		{
			CAT_draw_queue_add(cursor_sprite, 0, 2, spot.x, spot.y, CAT_DRAW_MODE_DEFAULT);
		}
	}
}