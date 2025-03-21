#include "cat_actions.h"

#include "cat_room.h"
#include "cat_input.h"
#include "cat_bag.h"
#include "cat_pet.h"
#include "cat_render.h"
#include <stdio.h>
#include <stddef.h>
#include "cat_item_dialog.h"

// MECHANIC PROFILE

static CAT_tool_type tool_type;

bool tool_filter(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	return 
	item->type == CAT_ITEM_TYPE_TOOL &&
	item->data.tool_data.type == tool_type;
}

static CAT_machine_state action_MS;
static CAT_animachine_state* action_AS;
static CAT_animachine_state* result_AS;

static uint8_t result_colour[3];

// MECHANIC STATE

static CAT_ivec2 cursor;
static int tool_id = -1;
static CAT_vec2 target_location;
static bool action_confirmed = false;
static bool action_complete = false;
static int timer_id = -1;

static void control_cursor()
{
	if(CAT_input_pulse(CAT_BUTTON_UP))
		cursor.y -= 1;
	if(CAT_input_pulse(CAT_BUTTON_RIGHT))
		cursor.x += 1;
	if(CAT_input_pulse(CAT_BUTTON_DOWN))
		cursor.y += 1;
	if(CAT_input_pulse(CAT_BUTTON_LEFT))
		cursor.x -= 1;
	cursor.x = clamp(cursor.x, 0, CAT_GRID_WIDTH-1);
	cursor.y = clamp(cursor.y, 0, CAT_GRID_HEIGHT-1);
}

void action_enter()
{
	if(tool_id == -1)
		cursor = CAT_largest_free_space();
	cursor = CAT_nearest_free_space(cursor);

	CAT_pet_settle();
}

void apply_tool()
{
	CAT_item* tool = CAT_item_get(tool_id);
	if(tool == NULL)
		return;
	
	pet.vigour = clamp(pet.vigour + tool->data.tool_data.dv, 0, 12);
	pet.focus = clamp(pet.focus + tool->data.tool_data.df, 0, 12);
	pet.spirit = clamp(pet.spirit + tool->data.tool_data.ds, 0, 12);
}

void action_tick()
{
	if(timer_id == -1)
		timer_id = CAT_timer_init(2.0f);
	
	if(tool_id == -1)
	{
		CAT_filter_item_dialog(tool_filter);
		CAT_target_item_dialog(&tool_id, true);
		CAT_machine_transition(CAT_MS_item_dialog);
	}
	else if(!action_confirmed)
	{
		if(tool_id == toy_laser_pointer_item)
			CAT_machine_transition(CAT_MS_laser);
			
		control_cursor();

		if(CAT_input_pressed(CAT_BUTTON_A))
		{
			CAT_ivec2 world_cursor = CAT_grid2world(cursor);
			CAT_rect action_rect = CAT_rect_place(cursor, (CAT_ivec2) {1, 1});
			if(CAT_is_block_free(action_rect))
			{
				int x_off = world_cursor.x > pet.pos.x ? -16 : 32;
				target_location = (CAT_vec2) {world_cursor.x + x_off, world_cursor.y + 16};

				action_confirmed = true;
				CAT_animachine_transition(&pet_asm, &AS_approach);
			}
		}
	}
	else
	{
		if(CAT_animachine_is_in(&pet_asm, &AS_approach) && CAT_animachine_is_ticking(&pet_asm))
		{
			if(CAT_pet_seek(target_location))
			{
				CAT_animachine_transition(&pet_asm, action_AS);
			}
		}
		if(CAT_animachine_is_in(&pet_asm, action_AS) && CAT_animachine_is_ticking(&pet_asm))
		{
			if(CAT_timer_tick(timer_id) || CAT_input_pressed(CAT_BUTTON_A))
			{
				apply_tool();
				CAT_pet_reanimate();
			
				CAT_item* item = CAT_item_get(tool_id);
				if(item->data.tool_data.type == CAT_TOOL_TYPE_FOOD)
				{
					CAT_item_list_remove(&bag, tool_id, 1);
				}
				
				action_complete = true;
				CAT_timer_reset(timer_id);

				CAT_animachine_kill(&pet_asm);
				CAT_animachine_transition(&pet_asm, result_AS);		
			}
		}
		if(CAT_animachine_is_in(&pet_asm, result_AS))
		{
			CAT_set_LEDs
			(
				result_colour[0],
				result_colour[1],
				result_colour[1]
			);
			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_animachine_kill(&pet_asm);
			if(CAT_animachine_is_done(&pet_asm))
				CAT_machine_transition(CAT_MS_room);
		}
	}

	if(CAT_input_pressed(CAT_BUTTON_B))
		CAT_machine_transition(CAT_MS_room);
}

void action_exit()
{
	tool_id = -1;
	action_confirmed = false;
	action_complete = false;

	CAT_timer_reset(timer_id);
	CAT_set_LEDs(0, 0, 0);
}

void CAT_MS_feed(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_action);
			tool_type = CAT_TOOL_TYPE_FOOD;
			action_MS = CAT_MS_feed;
			action_AS = &AS_eat;
			result_AS = &AS_vig_up;
			result_colour[0] = 255;
			result_colour[1] = 106;
			result_colour[2] = 171;
			action_enter();	
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
			CAT_set_render_callback(CAT_render_action);
			tool_type = CAT_TOOL_TYPE_BOOK;
			action_MS = CAT_MS_study;
			action_AS = &AS_study;
			result_AS = &AS_foc_up;
			result_colour[0] = 64;
			result_colour[1] = 206;
			result_colour[2] = 220;
			action_enter();
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
			CAT_set_render_callback(CAT_render_action);
			tool_type = CAT_TOOL_TYPE_TOY;
			action_MS = CAT_MS_play;
			action_AS = &AS_play;
			result_AS = &AS_spi_up;
			result_colour[0] = 76;
			result_colour[1] = 71;
			result_colour[2] = 255;
			action_enter();	
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

static CAT_vec2 laser_pos = {120, 180};
static CAT_vec2 laser_dir = {0, 0};
static float laser_speed = 72.0f;
static int play_timer_id = -1;
enum
{
	SEEKING,
	PLAYING,
	BORED
} laser_state = SEEKING;

void CAT_MS_laser(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_laser);
			CAT_pet_settle();
			play_timer_id = CAT_timer_init(CAT_rand_float(1.5f, 3.0f));
		break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
				laser_dir.x += 1.0f;
			if(CAT_input_held(CAT_BUTTON_UP, 0))
				laser_dir.y -= 1.0f;
			if(CAT_input_held(CAT_BUTTON_LEFT, 0))
				laser_dir.x -= 1.0f;
			if(CAT_input_held(CAT_BUTTON_DOWN, 0))
				laser_dir.y += 1.0f;		
			laser_pos = CAT_vec2_add(laser_pos, CAT_vec2_mul(laser_dir, laser_speed * CAT_get_delta_time()));
			laser_dir = (CAT_vec2) {0, 0};

			switch(laser_state)
			{
				case SEEKING:
					if(!CAT_animachine_is_in(&pet_asm, &AS_walk))
							CAT_animachine_transition(&pet_asm, &AS_walk);
					else if(CAT_pet_seek(laser_pos))
						laser_state = PLAYING;
				break;
				case PLAYING:
					if(!CAT_animachine_is_in(&pet_asm, &AS_play))
						CAT_animachine_transition(&pet_asm, &AS_play);
					if(CAT_timer_tick(play_timer_id))
					{
						CAT_timer_delete(play_timer_id);
						play_timer_id = CAT_timer_init(CAT_rand_float(1.0f, 3.0f));
						laser_state = BORED;
					}
					if(CAT_vec2_dist2(pet.pos, laser_pos) >= 16)
						laser_state = SEEKING;
				break;
				case BORED:
					if(!CAT_animachine_is_in(&pet_asm, &AS_crit))
						CAT_animachine_transition(&pet_asm, &AS_crit);
					if(CAT_vec2_dist2(pet.pos, laser_pos) >= 16)
						laser_state = SEEKING;
				break;
			}
		break;
		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_action()
{
	CAT_render_room();

	CAT_ivec2 place = CAT_grid2world(cursor);
	
	if(tool_id != -1)
	{
		CAT_item* item = CAT_item_get(tool_id);
		
		if(!action_confirmed)
		{
			CAT_draw_queue_add(item->data.tool_data.cursor, 0, 2, place.x, place.y+16, CAT_DRAW_MODE_BOTTOM);
			CAT_draw_queue_add(&tile_hl_sprite, 0, 3, place.x, place.y+16, CAT_DRAW_MODE_BOTTOM);
		}
		else if(!action_complete)
		{
			int tool_mode = CAT_DRAW_MODE_BOTTOM;
			if(place.x > pet.pos.x)
				tool_mode |= CAT_DRAW_MODE_REFLECT_X;
			int tool_layer = item->data.tool_data.type == CAT_TOOL_TYPE_FOOD ? 1 : 2;
			CAT_draw_queue_add(item->sprite, -1, tool_layer, place.x, place.y+16, tool_mode);
		}
	}
}

void CAT_render_laser()
{
	CAT_render_room();
	
	CAT_item* item = CAT_item_get(toy_laser_pointer_item);
	CAT_draw_queue_add(item->data.tool_data.cursor, 0, 0, laser_pos.x, laser_pos.y, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y);
}