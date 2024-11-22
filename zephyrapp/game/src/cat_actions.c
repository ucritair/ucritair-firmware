#include "cat_actions.h"

#include "cat_room.h"
#include "cat_input.h"
#include "cat_bag.h"
#include "cat_pet.h"
#include "cat_sprite.h"
#include <stdio.h>
#include <stddef.h>
#include "cat_item_dialog.h"

CAT_action_state action_state = 
{
	.action_MS = NULL,
	.action_AS = NULL,
	.stat_up_AS = NULL,

	.LED_colour = {0, 0, 0},
	
	.tool_type = CAT_ITEM_TYPE_KEY,
	.tool_id = -1,
	.confirmed = false,
	.complete = false,

	.timer_id = -1
};

void CAT_action_state_clear()
{
	action_state.tool_type = CAT_ITEM_TYPE_KEY;
	action_state.tool_id = -1;
	action_state.confirmed = false;
	action_state.complete = false;
	CAT_timer_reset(action_state.timer_id);
}

void CAT_action_tick()
{
	if(action_state.timer_id == -1)
	{
		action_state.timer_id = CAT_timer_init(2.0f);
	}
	
	if(action_state.tool_id == -1)
	{
		CAT_anchor_item_dialog(action_state.action_MS, action_state.tool_type, &action_state.tool_id);
		CAT_machine_transition(CAT_MS_item_dialog);
	}
	else if(!action_state.confirmed)
	{
		CAT_room_cursor();

		if(CAT_input_pressed(CAT_BUTTON_A))
		{
			CAT_ivec2 world_cursor = CAT_grid2world(room.grid_cursor);
			CAT_rect action_rect = CAT_rect_place(room.grid_cursor, (CAT_ivec2) {1, 1});
			if(CAT_block_free(action_rect))
			{
				int x_off = world_cursor.x > pet.pos.x ? -16 : 32;
				action_state.location = (CAT_vec2) {world_cursor.x + x_off, world_cursor.y + 16};

				action_state.confirmed = true;
				CAT_AM_transition(&pet_asm, &AS_adjust_in);
			}
		}
	}
	else
	{
		if(CAT_AM_is_in(&pet_asm, &AS_adjust_in))
		{
			CAT_AM_transition(&pet_asm, &AS_approach);
		}
		if(CAT_AM_is_in(&pet_asm, &AS_approach) && CAT_AM_is_ticking(&pet_asm))
		{
			if(CAT_pet_seek(action_state.location))
			{
				pet.left = (room.grid_cursor.x * 16) > pet.pos.x;
				CAT_AM_transition(&pet_asm, action_state.action_AS);
			}
		}
		if(CAT_AM_is_in(&pet_asm, action_state.action_AS) && CAT_AM_is_ticking(&pet_asm))
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

				CAT_AM_kill(&pet_asm);
				CAT_AM_transition(&pet_asm, action_state.stat_up_AS);		
			}
		}
		if(CAT_AM_is_in(&pet_asm, action_state.stat_up_AS))
		{
			CAT_set_LEDs(action_state.LED_colour[0], action_state.LED_colour[1], action_state.LED_colour[2]);
			CAT_AM_transition(&pet_asm, &AS_adjust_out);
		}	
		if(CAT_AM_is_in(&pet_asm, &AS_adjust_out))
		{
			CAT_machine_transition(CAT_MS_room);
		}
	}

	if(CAT_input_pressed(CAT_BUTTON_B))
	{
		CAT_machine_transition(CAT_MS_room);
	}	
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

			action_state.LED_colour[0] = 255;
			action_state.LED_colour[1] = 106;
			action_state.LED_colour[2] = 171;

			action_state.tool_type = CAT_ITEM_TYPE_FOOD;

			CAT_pet_settle();
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
			CAT_set_LEDs(0, 0, 0);
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

			action_state.LED_colour[0] = 74;
			action_state.LED_colour[1] = 206;
			action_state.LED_colour[2] = 220;

			action_state.tool_type = CAT_ITEM_TYPE_BOOK;

			CAT_pet_settle();
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
			CAT_set_LEDs(0, 0, 0);
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

			action_state.LED_colour[0] = 76;
			action_state.LED_colour[1] = 71;
			action_state.LED_colour[2] = 255;

			action_state.tool_type = CAT_ITEM_TYPE_TOY;

			CAT_pet_settle();
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
			CAT_set_LEDs(0, 0, 0);
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
			int tool_layer = item->data.tool_data.consumable ? 1 : 2;
			int tool_mode = CAT_DRAW_MODE_BOTTOM;

			if(!action_state.confirmed)
			{
				CAT_draw_queue_add(item->data.tool_data.cursor_sprite_id, 0, tool_layer, place.x, place.y+16, tool_mode);
			}			
			else if(!action_state.complete)
			{
				if(place.x > pet.pos.x)
					tool_mode |= CAT_DRAW_MODE_REFLECT_X;
				CAT_draw_queue_animate(item->sprite_id, tool_layer, place.x, place.y+16, tool_mode);
			}
		}
		else
		{
			CAT_draw_queue_add(cursor_sprite, 0, 2, place.x, place.y, CAT_DRAW_MODE_DEFAULT);
		}
	}
}