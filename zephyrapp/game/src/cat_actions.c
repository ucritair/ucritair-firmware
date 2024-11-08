#include "cat_actions.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_input.h"
#include "cat_bag.h"

CAT_action_state action_state;

void CAT_action_state_init()
{
	action_state.item_id = -1;
	action_state.confirmed = false;
}

void CAT_action_tick()
{
	CAT_room_move_cursor();

	if(CAT_input_pressed(CAT_BUTTON_B))
		CAT_machine_transition(&machine, CAT_MS_room);

	if(action_state.item_id == -1)
	{
		if(CAT_input_pressed(CAT_BUTTON_A))
		{
			bag_state.destination = action_state.action_MS;
			CAT_machine_transition(&machine, CAT_MS_bag);
		}
	}
	else if(!action_state.confirmed)
	{
		if(CAT_input_pressed(CAT_BUTTON_A))
		{
			CAT_ivec2 c_world = CAT_ivec2_mul(room.cursor, 16);
			int x_off = c_world.x > pet.pos.x ? -16 : 32;
			action_state.location = (CAT_vec2) {c_world.x + x_off, c_world.y + 16};
			action_state.confirmed = true;
			CAT_ASM_transition(&pet_asm, &AS_adjust_in);
		}
	}
	else
	{
		if(CAT_ASM_is_in(&pet_asm, &AS_adjust_in))
			CAT_ASM_transition(&pet_asm, &AS_walk_action);
		if(CAT_ASM_is_in(&pet_asm, &AS_walk_action) && CAT_ASM_is_ticking(&pet_asm))
		{
			if(CAT_pet_seek(action_state.location))
			{
				pet.left = (room.cursor.x * 16) > pet.pos.x;
				CAT_ASM_transition(&pet_asm, action_state.action_AS);
			}
		}
		if(CAT_ASM_is_in(&pet_asm, action_state.action_AS))
		{
			if(CAT_timer_tick(pet.action_timer_id))
			{
				action_state.action_proc();
				CAT_ASM_kill(&pet_asm);
				CAT_ASM_transition(&pet_asm, action_state.stat_up_AS);
				CAT_timer_reset(pet.action_timer_id);
			}
		}
		if(CAT_ASM_is_in(&pet_asm, action_state.stat_up_AS))
			CAT_ASM_transition(&pet_asm, &AS_adjust_out);
		if(CAT_ASM_is_in(&pet_asm, &AS_adjust_out))
			CAT_machine_transition(&machine, CAT_MS_room);
	}
}

void CAT_feed_proc()
{
	pet.vigour += 3;
}

void CAT_MS_feed(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			action_state.action_MS = CAT_MS_feed;
			action_state.action_proc = CAT_feed_proc;
			action_state.action_AS = &AS_eat;
			action_state.stat_up_AS = &AS_vig_up;
			CAT_ASM_transition(&pet_asm, &AS_idle);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_action_tick();
			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			CAT_action_state_init();
			break;
		}
	}
}

void CAT_study_proc()
{
	pet.focus += 3;
}

void CAT_MS_study(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			action_state.action_MS = CAT_MS_study;
			action_state.action_proc = CAT_study_proc;
			action_state.action_AS = &AS_study;
			action_state.stat_up_AS = &AS_foc_up;
			CAT_ASM_transition(&pet_asm, &AS_idle);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_action_tick();
			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			CAT_action_state_init();
			break;
		}
	}
}

void CAT_play_proc()
{
	pet.spirit += 3;
}

void CAT_MS_play(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			action_state.action_MS = CAT_MS_play;
			action_state.action_proc = CAT_play_proc;
			action_state.action_AS = &AS_play;
			action_state.stat_up_AS = &AS_spi_up;
			CAT_ASM_transition(&pet_asm, &AS_idle);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_action_tick();
			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			CAT_action_state_init();
			break;
		}
	}
}

void CAT_render_action()
{
	if(action_state.item_id != -1)
	{
		CAT_item* item = &item_table.data[action_state.item_id];
		CAT_draw_queue_add(item->sprite_id, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
		if(!action_state.confirmed)
			CAT_draw_queue_add(tile_hl_sprite, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
	}
	else
	{
		CAT_draw_queue_add(cursor_sprite, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
	}
}