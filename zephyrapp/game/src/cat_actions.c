#include "cat_actions.h"

#include "cat_room.h"
#include "cat_input.h"
#include "cat_bag.h"
#include "cat_pet.h"
#include "cat_sprite.h"
#include <stdio.h>
#include <stddef.h>
#include "cat_item_dialog.h"

CAT_action_profile feed_profile =
{
	.tool_type = CAT_ITEM_TYPE_FOOD,

	.MS = CAT_MS_feed,
	.AS = &AS_eat,
	.stat_AS = &AS_vig_up,

	.LED_colour = {255, 106, 171}
};
CAT_action_profile study_profile =
{
	.tool_type = CAT_ITEM_TYPE_BOOK,

	.MS = CAT_MS_study,
	.AS = &AS_study,
	.stat_AS = &AS_foc_up,

	.LED_colour = {64, 206, 220}
};
CAT_action_profile play_profile =
{
	.tool_type = CAT_ITEM_TYPE_TOY,

	.MS = CAT_MS_play,
	.AS = &AS_play,
	.stat_AS = &AS_spi_up,

	.LED_colour = {76, 71, 255}
};

CAT_action_state action_state = 
{
	.profile = NULL,

	.tool_id = -1,
	.confirmed = false,
	.complete = false,

	.timer_id = -1
};

void action_enter(CAT_action_profile* profile)
{
	room.grid_cursor = CAT_first_free_space();
	CAT_pet_settle();
	action_state.profile = profile;
}

void action_tick()
{
	if(action_state.timer_id == -1)
	{
		action_state.timer_id = CAT_timer_init(2.0f);
	}
	
	if(action_state.tool_id == -1)
	{
		CAT_anchor_item_dialog(action_state.profile->MS, action_state.profile->tool_type, &action_state.tool_id);
		CAT_machine_transition(CAT_MS_item_dialog);
	}
	else if(!action_state.confirmed)
	{
		CAT_room_cursor();

		if(CAT_input_pressed(CAT_BUTTON_A))
		{
			CAT_ivec2 world_cursor = CAT_grid2world(room.grid_cursor);
			CAT_rect action_rect = CAT_rect_place(room.grid_cursor, (CAT_ivec2) {1, 1});
			if(CAT_is_block_free(action_rect))
			{
				int x_off = world_cursor.x > pet.pos.x ? -16 : 32;
				action_state.location = (CAT_vec2) {world_cursor.x + x_off, world_cursor.y + 16};

				action_state.confirmed = true;
				CAT_animachine_transition(&pet_asm, &AS_adjust_in);
			}
		}
	}
	else
	{
		if(CAT_animachine_is_in(&pet_asm, &AS_adjust_in))
		{
			CAT_animachine_transition(&pet_asm, &AS_approach);
		}
		if(CAT_animachine_is_in(&pet_asm, &AS_approach) && CAT_animachine_is_ticking(&pet_asm))
		{
			if(CAT_pet_seek(action_state.location))
			{
				pet.left = (room.grid_cursor.x * 16) > pet.pos.x;
				CAT_animachine_transition(&pet_asm, action_state.profile->AS);
			}
		}
		if(CAT_animachine_is_in(&pet_asm, action_state.profile->AS) && CAT_animachine_is_ticking(&pet_asm))
		{
			if(CAT_timer_tick(action_state.timer_id) || CAT_input_pressed(CAT_BUTTON_A))
			{
				CAT_pet_use(action_state.tool_id);
				CAT_pet_reanimate();
			
				CAT_item* item = CAT_item_get(action_state.tool_id);
				if(item->data.tool_data.consumable)
				{
					CAT_item_list_remove(&bag, action_state.tool_id);
				}
				
				action_state.complete = true;
				CAT_timer_reset(action_state.timer_id);

				CAT_animachine_kill(&pet_asm);
				CAT_animachine_transition(&pet_asm, action_state.profile->stat_AS);		
			}
		}
		if(CAT_animachine_is_in(&pet_asm, action_state.profile->stat_AS))
		{
			CAT_set_LEDs(action_state.profile->LED_colour[0], action_state.profile->LED_colour[0], action_state.profile->LED_colour[0]);
			CAT_animachine_transition(&pet_asm, &AS_adjust_out);
		}	
		if(CAT_animachine_is_in(&pet_asm, &AS_adjust_out))
		{
			CAT_machine_transition(CAT_MS_room);
		}
	}

	if(CAT_input_pressed(CAT_BUTTON_B))
	{
		CAT_machine_transition(CAT_MS_room);
	}	
}

void action_exit()
{
	action_state.tool_id = -1;
	action_state.confirmed = false;
	action_state.complete = false;
	
	CAT_timer_reset(action_state.timer_id);
	CAT_set_LEDs(0, 0, 0);
}

void CAT_MS_feed(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			action_enter(&feed_profile);	
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			action_tick();
			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			action_exit();
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
			action_enter(&study_profile);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			action_tick();
			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			action_exit();
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
			action_enter(&play_profile);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			action_tick();
			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			action_exit();
			break;
		}
	}
}

void CAT_render_action(int cycle)
{
	if(cycle == 0)
	{
		CAT_ivec2 place = CAT_grid2world(room.grid_cursor);
		
		if(action_state.tool_id != -1)
		{
			CAT_item* item = CAT_item_get(action_state.tool_id);
			
			if(!action_state.confirmed)
			{
				CAT_draw_queue_add(item->data.tool_data.cursor_sprite_id, 0, 2, place.x, place.y+16, CAT_DRAW_MODE_BOTTOM);
				CAT_draw_queue_add(tile_hl_sprite, 0, 3, place.x, place.y+16, CAT_DRAW_MODE_BOTTOM);
			}			
			else if(!action_state.complete)
			{
				int tool_mode = CAT_DRAW_MODE_BOTTOM;
				if(place.x > pet.pos.x)
					tool_mode |= CAT_DRAW_MODE_REFLECT_X;
				int tool_layer = item->data.tool_data.consumable ? 1 : 2;
				CAT_draw_queue_animate(item->sprite_id, tool_layer, place.x, place.y+16, tool_mode);
			}
		}
	}
}