#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cat_core.h"
#include "cat_sprite.h"
#include "cat_math.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"

#pragma region CONSTANTS

#define CAT_MAX_PROP_COUNT 210

#pragma endregion

#pragma region DATA

CAT_machine_state machine = NULL;
void CAT_MS_default(CAT_machine_signal);
void CAT_MS_feed(CAT_machine_signal);
void CAT_MS_study(CAT_machine_signal);
void CAT_MS_play(CAT_machine_signal);
void CAT_MS_deco(CAT_machine_signal);
void CAT_MS_menu(CAT_machine_signal);
void CAT_MS_stats(CAT_machine_signal);
void CAT_MS_bag(CAT_machine_signal);
void CAT_MS_manual(CAT_machine_signal);

CAT_ASM_state* pet_asm = NULL;
CAT_ASM_state AS_idle;
CAT_ASM_state AS_walk;
CAT_ASM_state AS_adjust_in;
CAT_ASM_state AS_walk_action;
CAT_ASM_state AS_eat;
CAT_ASM_state AS_study;
CAT_ASM_state AS_play;
CAT_ASM_state AS_adjust_out;
CAT_ASM_state AS_vig_up;
CAT_ASM_state AS_foc_up;
CAT_ASM_state AS_spi_up;

CAT_ASM_state* bubl_asm = NULL;
CAT_ASM_state AS_react;

typedef struct CAT_deco_state
{
	int props[CAT_MAX_PROP_COUNT];
	CAT_ivec2 places[CAT_MAX_PROP_COUNT];
	int prop_count;

	enum mode {ADD, FLIP, REMOVE} mode;

	int add_id;
	CAT_rect add_rect;
	bool valid_add;

	int mod_idx;
	CAT_rect mod_rect;
} CAT_deco_state;
CAT_deco_state deco_state;

typedef struct CAT_menu_state
{
	const char* items[4];
	int selector;
} CAT_menu_state;
CAT_menu_state menu_state;

typedef struct CAT_bag_state
{
	int base;
	int idx;
	CAT_machine_state destination;
} CAT_bag_state;
CAT_bag_state bag_state;

#pragma endregion

#pragma region ROOM

typedef struct CAT_room
{
	CAT_rect bounds;
	CAT_ivec2 cursor;

	CAT_machine_state buttons[5];
	int selector;

	CAT_vec2 poi;
} CAT_room;
CAT_room room;

void CAT_room_move_cursor()
{
	if(CAT_input_pulse(CAT_BUTTON_UP))
		room.cursor.y -= 1;
	if(CAT_input_pulse(CAT_BUTTON_RIGHT))
		room.cursor.x += 1;
	if(CAT_input_pulse(CAT_BUTTON_DOWN))
		room.cursor.y += 1;
	if(CAT_input_pulse(CAT_BUTTON_LEFT))
		room.cursor.x -= 1;
	room.cursor = CAT_clamp_pt_rect(room.cursor, room.bounds);
}

void CAT_room_init()
{
	room.bounds = (CAT_rect){{0, 7}, {15, 17}};
	room.cursor = room.bounds.min;

	room.buttons[0] = CAT_MS_feed;
	room.buttons[1] = CAT_MS_study;
	room.buttons[2] = CAT_MS_play;
	room.buttons[3] = CAT_MS_deco;
	room.buttons[4] = CAT_MS_menu;
	room.selector = 0;
}

#pragma endregion

#pragma region PET

typedef struct CAT_pet
{
	float vigour;
	float focus;
	float spirit;
	bool critical;

	CAT_vec2 pos;
	CAT_vec2 dir;
	bool left;
	
	int stat_timer_id;
	int walk_timer_id;
	int react_timer_id;
	int action_timer_id;
} CAT_pet;
CAT_pet pet;

bool CAT_pet_seek(CAT_vec2 targ)
{
	CAT_vec2 line = CAT_vec2_sub(targ, pet.pos);
	float dist = CAT_vec2_mag(line);
	float step = 48.0f * CAT_get_delta_time();
	if(dist < step)
	{
		pet.pos = targ;
		pet.dir = (CAT_vec2) {0, 0};
		return true;
	}
	else
	{
		pet.dir = CAT_vec2_div(line, dist);
		pet.pos = CAT_vec2_add(pet.pos, CAT_vec2_mul(pet.dir, step));
		pet.left = pet.pos.x < targ.x;
		return false;
	}
}

void CAT_pet_transition(int sprite_id)
{
	return;
}

void CAT_pet_stat()
{
	float dv = -1;
	float df = -1;
	float ds = -1;

	pet.vigour += dv;
	pet.focus += df;
	pet.spirit += ds;
	pet.critical = (pet.vigour >= 1 && pet.focus >= 1 && pet.spirit >= 1);
}

void CAT_pet_anim_init()
{
	CAT_ASM_init(&AS_idle, -1, pet_idle_high_vig_sprite, -1);
	CAT_ASM_init(&AS_walk, -1, pet_walk_high_vig_sprite, -1);
	CAT_ASM_init(&AS_adjust_in, -1, -1, pet_wings_in_sprite);
	CAT_ASM_init(&AS_walk_action, -1, pet_walk_sprite, -1);
	CAT_ASM_init(&AS_eat, pet_eat_down_sprite, pet_chew_sprite, pet_eat_up_sprite);
	CAT_ASM_init(&AS_study, pet_eat_down_sprite, pet_chew_sprite, pet_eat_up_sprite);
	CAT_ASM_init(&AS_play, pet_eat_down_sprite, pet_chew_sprite, pet_eat_up_sprite);
	CAT_ASM_init(&AS_adjust_out, -1, -1, pet_wings_out_sprite);
	CAT_ASM_init(&AS_vig_up, -1, -1, pet_vig_up_sprite);
	CAT_ASM_init(&AS_foc_up, -1, -1, pet_foc_up_sprite);
	CAT_ASM_init(&AS_spi_up, -1, -1, pet_spi_up_sprite);

	CAT_ASM_init(&AS_react, -1, bubl_react_good_sprite, -1);
}

void CAT_pet_init()
{
	pet.vigour = 12;
	pet.focus = 12;
	pet.spirit = 12;

	pet.pos = (CAT_vec2) {120, 200};
	pet.dir = (CAT_vec2) {0, 0};
	pet.left = false;
	
	pet.stat_timer_id = CAT_timer_init(5.0f);
	pet.walk_timer_id = CAT_timer_init(4.0f);
	pet.react_timer_id = CAT_timer_init(2.0f);
	pet.action_timer_id = CAT_timer_init(2.0f);

	CAT_pet_anim_init();
}

#pragma endregion

#pragma region DEFAULT

void CAT_MS_default(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_ASM_transition(&pet_asm, &AS_idle);
			CAT_ASM_transition(&bubl_asm, NULL);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				room.selector += 1;
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				room.selector -= 1;
			room.selector = clamp(room.selector, 0, 4);
			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_machine_transition(&machine, room.buttons[room.selector]);

			if(CAT_input_touch(pet.pos.x, pet.pos.y-16, 16))
			{
				CAT_ASM_transition(&bubl_asm, &AS_react);
			}
			if(CAT_ASM_is_in(&bubl_asm, &AS_react))
			{
				if(CAT_timer_tick(pet.react_timer_id))
				{
					CAT_timer_reset(pet.react_timer_id);
					CAT_ASM_transition(&bubl_asm, NULL);
				}
			}

			if(CAT_ASM_is_in(&pet_asm, &AS_idle) && CAT_ASM_is_ticking(&pet_asm))
			{
				if(CAT_timer_tick(pet.walk_timer_id))
				{
					CAT_ivec2 grid_min = CAT_ivec2_add(room.bounds.min, (CAT_ivec2){1, 1});
					CAT_ivec2 grid_max = CAT_ivec2_add(room.bounds.max, (CAT_ivec2){-1, -1});
					CAT_vec2 world_min = CAT_iv2v(CAT_ivec2_mul(grid_min, 16));
					CAT_vec2 world_max = CAT_iv2v(CAT_ivec2_mul(grid_max, 16));
					room.poi = CAT_rand_vec2(world_min, world_max);

					CAT_ASM_transition(&pet_asm, &AS_walk);
					CAT_timer_reset(pet.walk_timer_id);
				}
			}

			if(CAT_ASM_is_in(&pet_asm, &AS_walk) && CAT_ASM_is_ticking(&pet_asm))
			{
				if(CAT_pet_seek(room.poi))
				{
					CAT_ASM_transition(&pet_asm, &AS_idle);
				}
			}

			if(CAT_timer_tick(pet.stat_timer_id))
			{
				CAT_pet_stat();
				CAT_timer_reset(pet.stat_timer_id);
			}
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

#pragma endregion

#pragma region FEED

typedef struct CAT_feed_state
{
	CAT_vec2 location;
	bool confirmed;
	int food_id;
} CAT_feed_state;
CAT_feed_state feed_state;

void CAT_feed_state_init()
{
	feed_state.food_id = -1;
	feed_state.confirmed = false;
}

void CAT_MS_feed(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(!feed_state.confirmed)
			{
				if(feed_state.food_id == -1)
				{
					CAT_room_move_cursor();
					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						bag_state.destination = CAT_MS_feed;
						CAT_machine_transition(&machine, CAT_MS_bag);
					}
				}
				else
				{
					CAT_room_move_cursor();
					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						CAT_ivec2 c_world = CAT_ivec2_mul(room.cursor, 16);
						int x_off = c_world.x > pet.pos.x ? -16 : 32;
						feed_state.location = (CAT_vec2) {c_world.x + x_off, c_world.y + 16};
						feed_state.confirmed = true;
						CAT_ASM_transition(&pet_asm, &AS_adjust_in);
					}
				}

				if(CAT_input_pressed(CAT_BUTTON_B))
					CAT_machine_transition(&machine, CAT_MS_default);
			}
			
			
			if(CAT_ASM_is_in(&pet_asm, &AS_adjust_in))
				CAT_ASM_transition(&pet_asm, &AS_walk_action);
			if(CAT_ASM_is_in(&pet_asm, &AS_walk_action) && CAT_ASM_is_ticking(&pet_asm))
			{
				if(CAT_pet_seek(feed_state.location))
				{
					pet.left = (room.cursor.x * 16) > pet.pos.x;
					CAT_ASM_transition(&pet_asm, &AS_eat);
				}
			}
			if(CAT_ASM_is_in(&pet_asm, &AS_eat))
			{
				if(CAT_timer_tick(pet.action_timer_id))
				{
					CAT_item* item = CAT_item_get(feed_state.food_id);
					pet.vigour += item->data.food_data.d_v;
					pet.focus += item->data.food_data.d_f;
					pet.spirit += item->data.food_data.d_s;

					CAT_ASM_kill(&pet_asm);
					CAT_ASM_transition(&pet_asm, &AS_vig_up);
					CAT_timer_reset(pet.action_timer_id);
				}
			}
			if(CAT_ASM_is_in(&pet_asm, &AS_vig_up))
				CAT_ASM_transition(&pet_asm, &AS_adjust_out);
			if(CAT_ASM_is_in(&pet_asm, &AS_adjust_out))
				CAT_machine_transition(&machine, CAT_MS_default);

			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			feed_state.food_id = -1;
			feed_state.confirmed = false;
			break;
		}
	}
}

#pragma endregion

#pragma region STUDY

typedef struct CAT_study_state
{
	CAT_vec2 location;
	bool confirmed;
	int book_id;
} CAT_study_state;
CAT_study_state study_state;

void CAT_study_state_init()
{
	study_state.book_id = -1;
	study_state.confirmed = false;
}

void CAT_MS_study(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(!study_state.confirmed)
			{
				if(study_state.book_id == -1)
				{
					CAT_room_move_cursor();
					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						bag_state.destination = CAT_MS_study;
						CAT_machine_transition(&machine, CAT_MS_bag);
					}
				}
				else
				{
					CAT_room_move_cursor();
					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						CAT_ivec2 c_world = CAT_ivec2_mul(room.cursor, 16);
						int x_off = c_world.x > pet.pos.x ? -16 : 32;
						study_state.location = (CAT_vec2) {c_world.x + x_off, c_world.y + 16};
						study_state.confirmed = true;
						CAT_ASM_transition(&pet_asm, &AS_adjust_in);
					}
				}

				if(CAT_input_pressed(CAT_BUTTON_B))
					CAT_machine_transition(&machine, CAT_MS_default);
			}
			
			if(CAT_ASM_is_in(&pet_asm, &AS_adjust_in))
				CAT_ASM_transition(&pet_asm, &AS_walk_action);
			if(CAT_ASM_is_in(&pet_asm, &AS_walk_action) && CAT_ASM_is_ticking(&pet_asm))
			{
				if(CAT_pet_seek(study_state.location))
				{
					pet.left = (room.cursor.x * 16) > pet.pos.x;
					CAT_ASM_transition(&pet_asm, &AS_study);
				}
			}
			if(CAT_ASM_is_in(&pet_asm, &AS_study))
			{
				if(CAT_timer_tick(pet.action_timer_id))
				{
					pet.focus += 3;

					CAT_ASM_kill(&pet_asm);
					CAT_ASM_transition(&pet_asm, &AS_vig_up);
					CAT_timer_reset(pet.action_timer_id);
				}
			}
			if(CAT_ASM_is_in(&pet_asm, &AS_vig_up))
				CAT_ASM_transition(&pet_asm, &AS_adjust_out);
			if(CAT_ASM_is_in(&pet_asm, &AS_adjust_out))
				CAT_machine_transition(&machine, CAT_MS_default);

			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			study_state.book_id = -1;
			study_state.confirmed = false;
			break;
		}
	}
}

#pragma endregion

#pragma region PLAY

typedef struct CAT_play_state
{
	CAT_vec2 location;
	bool confirmed;
	int toy_id;
} CAT_play_state;
CAT_play_state play_state;

void CAT_play_state_init()
{
	play_state.toy_id = -1;
	play_state.confirmed = false;
}

void CAT_MS_play(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(!play_state.confirmed)
			{
				if(play_state.toy_id == -1)
				{
					CAT_room_move_cursor();
					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						bag_state.destination = CAT_MS_play;
						CAT_machine_transition(&machine, CAT_MS_bag);
					}
				}
				else
				{
					CAT_room_move_cursor();
					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						CAT_ivec2 c_world = CAT_ivec2_mul(room.cursor, 16);
						int x_off = c_world.x > pet.pos.x ? -16 : 32;
						play_state.location = (CAT_vec2) {c_world.x + x_off, c_world.y + 16};
						play_state.confirmed = true;
						CAT_ASM_transition(&pet_asm, &AS_adjust_in);
					}
				}

				if(CAT_input_pressed(CAT_BUTTON_B))
					CAT_machine_transition(&machine, CAT_MS_default);
			}
			
			
			if(CAT_ASM_is_in(&pet_asm, &AS_adjust_in))
				CAT_ASM_transition(&pet_asm, &AS_walk_action);
			if(CAT_ASM_is_in(&pet_asm, &AS_walk_action) && CAT_ASM_is_ticking(&pet_asm))
			{
				if(CAT_pet_seek(play_state.location))
				{
					pet.left = (room.cursor.x * 16) > pet.pos.x;
					CAT_ASM_transition(&pet_asm, &AS_play);
				}
			}
			if(CAT_ASM_is_in(&pet_asm, &AS_play))
			{
				if(CAT_timer_tick(pet.action_timer_id))
				{
					pet.focus += 3;

					CAT_ASM_kill(&pet_asm);
					CAT_ASM_transition(&pet_asm, &AS_vig_up);
					CAT_timer_reset(pet.action_timer_id);
				}
			}
			if(CAT_ASM_is_in(&pet_asm, &AS_vig_up))
				CAT_ASM_transition(&pet_asm, &AS_adjust_out);
			if(CAT_ASM_is_in(&pet_asm, &AS_adjust_out))
				CAT_machine_transition(&machine, CAT_MS_default);

			break;	
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			play_state.toy_id = -1;
			play_state.confirmed = false;
			break;
		}
	}
}

#pragma endregion

#pragma region DECO

void CAT_deco_state_init()
{
	deco_state.prop_count = 0;
	deco_state.mode = ADD;
	deco_state.add_id = -1;
	deco_state.mod_idx = -1;
}

void CAT_deco_select()
{
	for(int i = 0; i < deco_state.prop_count; i++)
	{
		int item_id = deco_state.props[i];
		CAT_item* prop = &item_table.data[item_id];
		CAT_ivec2 shape = prop->data.prop_data.shape;
		CAT_rect bounds = CAT_rect_place(deco_state.places[i], shape);
		bounds.max = CAT_ivec2_add(bounds.max, (CAT_ivec2) {-1, -1});

		if(CAT_test_pt_rect(room.cursor, bounds))
		{
			deco_state.mod_idx = i;
			deco_state.mod_rect = bounds;
			return;
		}
	}

	deco_state.mod_idx = -1;
	deco_state.mod_rect = (CAT_rect) {{0, 0}, {0, 0}};
}

void CAT_MS_deco(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_pet_transition(pet_idle_sprite);
			deco_state.mode = ADD;
			deco_state.mod_idx = -1;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_room_move_cursor();

			if(CAT_input_pressed(CAT_BUTTON_SELECT))
			{
				deco_state.mode += 1;
				if(deco_state.mode > REMOVE)
					deco_state.mode = ADD;
			}

			switch(deco_state.mode)
			{
				case ADD:
				{
					if(deco_state.add_id != -1)
					{
						CAT_item* prop = &item_table.data[deco_state.add_id];
						CAT_ivec2 shape = prop->data.prop_data.shape;
						CAT_rect bounds = CAT_rect_place(room.cursor, shape);
						if(!CAT_test_contains(room.bounds, bounds))
						{
							if(bounds.max.x > room.bounds.max.x)
								room.cursor.x -= bounds.max.x - room.bounds.max.x;
							if(bounds.max.y > room.bounds.max.y)
								room.cursor.y -= bounds.max.y - room.bounds.max.y;
						}
						bounds = CAT_rect_place(room.cursor, shape);

						deco_state.valid_add = true;
						for(int i = 0; i < deco_state.prop_count && deco_state.valid_add; i++)
						{
							CAT_item* other = CAT_item_get(deco_state.props[i]);
							CAT_ivec2 other_shape = other->data.prop_data.shape;
							CAT_rect other_bounds = CAT_rect_place(deco_state.places[i], other_shape);
							
							if(CAT_test_overlaps(bounds, other_bounds))
							{
								deco_state.valid_add = false;
								break;
							}
						}
						deco_state.add_rect = bounds;
						
						if(CAT_input_pressed(CAT_BUTTON_A) && deco_state.valid_add)
						{
							int idx = deco_state.prop_count;
							deco_state.prop_count += 1;
							deco_state.props[idx] = deco_state.add_id;
							deco_state.places[idx] = room.cursor;

							CAT_bag_remove(deco_state.add_id);
							deco_state.add_id = -1;
						}
					}
					else
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							bag_state.destination = CAT_MS_deco;
							CAT_machine_transition(&machine, CAT_MS_bag);
						}
					}
					break;
				}
				case FLIP:
				{
					CAT_deco_select();
					if(deco_state.mod_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							int item_id = deco_state.props[deco_state.mod_idx];
							CAT_prop_flip(item_id);
						}
					}
					break;
				}
				case REMOVE:
				{
					CAT_deco_select();
					if(deco_state.mod_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							int item_id = deco_state.props[deco_state.mod_idx];
							CAT_bag_add(item_id);

							for(int i = deco_state.mod_idx; i < deco_state.prop_count-1; i++)
							{
								deco_state.props[i] = deco_state.props[i+1];
								deco_state.places[i] = deco_state.places[i+1];
							}
							deco_state.prop_count -= 1;

							deco_state.mod_idx = -1;
						}
					}
					break;
				}
			}
		
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_default);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			deco_state.add_id = -1;
			break;
		}
	}
}

#pragma endregion

#pragma region MENU

void CAT_menu_state_init()
{
	menu_state.items[0] = "INSIGHTS";
	menu_state.items[1] = "BAG";
	menu_state.items[2] = "CONTROLS";
	menu_state.items[3] = "BACK";
	menu_state.selector = 0;
}

void CAT_MS_menu(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pulse(CAT_BUTTON_UP))
				menu_state.selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				menu_state.selector += 1;
			menu_state.selector = clamp(menu_state.selector, 0, 3);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(menu_state.selector == 0)
					CAT_machine_transition(&machine, CAT_MS_stats);
				if(menu_state.selector == 1)
					CAT_machine_transition(&machine, CAT_MS_bag);
				if(menu_state.selector == 2)
					CAT_machine_transition(&machine, CAT_MS_manual);
				if(menu_state.selector == 3)
					CAT_machine_transition(&machine, CAT_MS_default);
			}

			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_default);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

#pragma endregion

#pragma region STATS

void CAT_MS_stats(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_menu);
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_default);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

#pragma endregion

#pragma region BAG

void CAT_bag_state_init()
{
	bag_state.base = 0;
	bag_state.idx = 0;
	bag_state.destination = CAT_MS_menu;
}

void CAT_MS_bag(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			bag_state.base = 0;
			bag_state.idx = 0;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, bag_state.destination);
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_default);

			if(bag.length <= 0)
				break;

			if(CAT_input_pulse(CAT_BUTTON_UP))
			{
				bag_state.idx -= 1;
				if(bag_state.idx == -1)
					bag_state.idx = bag.length-1;
			}
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
			{
				bag_state.idx += 1;
				if(bag_state.idx == bag.length)
					bag_state.idx = 0;
			}
				
			bag_state.idx = clamp(bag_state.idx, 0, bag.length-1);

			int overshoot = bag_state.idx - bag_state.base;
			if(overshoot < 0)
				bag_state.base += overshoot;
			else if(overshoot >= 9)
				bag_state.base += (overshoot - 8);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				int item_id = bag.item_id[bag_state.idx];
				CAT_item* item = &item_table.data[item_id];

				if(item->type == CAT_ITEM_TYPE_FOOD && (bag_state.destination == CAT_MS_menu || bag_state.destination == CAT_MS_feed))
				{
					feed_state.food_id = item_id;
					CAT_machine_transition(&machine, CAT_MS_feed);
				}
				else if(item->type == CAT_ITEM_TYPE_PROP && (bag_state.destination == CAT_MS_menu || bag_state.destination == CAT_MS_deco))
				{
					deco_state.add_id = item_id;
					CAT_machine_transition(&machine, CAT_MS_deco);
				}
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			bag_state.destination = CAT_MS_menu;
			break;
	}
}

#pragma endregion

#pragma region STATS

void CAT_MS_manual(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_menu);
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_default);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

#pragma endregion

#pragma region RENDER

void CAT_render(int cycle)
{
	if
	(
		machine == CAT_MS_default ||
		machine == CAT_MS_feed ||
		machine == CAT_MS_study ||
		machine == CAT_MS_play ||
		machine == CAT_MS_deco
	)
	{
		CAT_draw_tiles(base_wall_sprite, 0, 0, 4);
		CAT_draw_tiles(base_wall_sprite, 1, 4, 1);
		CAT_draw_tiles(base_wall_sprite, 2, 5, 1);
		CAT_draw_tiles(base_floor_sprite, 0, 6, 1);
		CAT_draw_tiles(base_floor_sprite, 1, 7, 10);
		CAT_draw_tiles(base_floor_sprite, 2, 17, 3);

		CAT_draw_queue_add(window_day_sprite, 0, 2, 8, 8, CAT_DRAW_MODE_DEFAULT);
		CAT_draw_queue_animate(vending_sprite, 2, 164, 112, CAT_DRAW_MODE_BOTTOM);
		
		for(int i = 0; i < deco_state.prop_count; i++)
		{
			int prop_id = deco_state.props[i];
			CAT_item* prop = CAT_item_get(prop_id);
			CAT_ivec2 shape = prop->data.prop_data.shape;
			CAT_ivec2 place = deco_state.places[i];
			if(prop->data.prop_data.animate)
				CAT_draw_queue_animate(prop->sprite_id, 2, place.x * 16, (place.y+shape.y) * 16, CAT_DRAW_MODE_BOTTOM);
			else
			{
				int frame_idx = prop->data.prop_data.frame_idx;
				CAT_draw_queue_add(prop->sprite_id, frame_idx, 2, place.x * 16, (place.y+shape.y) * 16, CAT_DRAW_MODE_BOTTOM);
			}
		}

		int pet_mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;
		if(pet.left)
			pet_mode |= CAT_DRAW_MODE_REFLECT_X;
		CAT_draw_queue_animate(CAT_ASM_tick(&pet_asm), 2, pet.pos.x, pet.pos.y, pet_mode);	
		if(bubl_asm != NULL)
		{
			int x_off = pet.left ? 16 : -16;
			CAT_draw_queue_animate(CAT_ASM_tick(&bubl_asm), 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);	
		}

		if(machine == CAT_MS_feed)
		{
			if(feed_state.food_id != -1)
			{
				CAT_item* food = &item_table.data[feed_state.food_id];
				CAT_draw_queue_add(food->sprite_id, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
				if(!feed_state.confirmed)
					CAT_draw_queue_add(tile_hl_sprite, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}
			else
			{
				CAT_draw_queue_add(cursor_sprite, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}
		}
		if(machine == CAT_MS_study)
		{
			if(study_state.book_id != -1)
			{
				CAT_item* book = &item_table.data[study_state.book_id];
				CAT_draw_queue_add(book->sprite_id, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
				if(!study_state.confirmed)
					CAT_draw_queue_add(tile_hl_sprite, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}
			else
			{
				CAT_draw_queue_add(cursor_sprite, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}
		}
		if(machine == CAT_MS_play)
		{
			if(play_state.toy_id != -1)
			{
				CAT_item* toy = &item_table.data[play_state.toy_id];
				CAT_draw_queue_add(toy->sprite_id, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
				if(!play_state.confirmed)
					CAT_draw_queue_add(tile_hl_sprite, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}
			else
			{
				CAT_draw_queue_add(cursor_sprite, 0, 2, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}
		}
		else if(machine == CAT_MS_deco)
		{
			switch(deco_state.mode)
			{
				case ADD:
				{
					if(deco_state.add_id != -1)
					{
						int tile_sprite = deco_state.valid_add ? tile_hl_add_sprite : tile_hl_rm_sprite;
						for(int y = deco_state.add_rect.min.y; y < deco_state.add_rect.max.y; y++)
						{
							for(int x = deco_state.add_rect.min.x; x < deco_state.add_rect.max.x; x++)
							{
								CAT_draw_queue_add(tile_sprite, 0, 3, x * 16, y * 16, CAT_DRAW_MODE_DEFAULT);
							}
						}
					}
					else
					{
						CAT_draw_queue_add(cursor_add_sprite, 0, 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
					}
					break;
				}
				case FLIP:
				{
					if(deco_state.mod_idx != -1)
					{
						for(int y = deco_state.mod_rect.min.y; y <= deco_state.mod_rect.max.y; y++)
						{
							for(int x = deco_state.mod_rect.min.x; x <= deco_state.mod_rect.max.x; x++)
							{
								CAT_draw_queue_add(tile_hl_flip_sprite, 0, 3, x * 16, y * 16, CAT_DRAW_MODE_DEFAULT);
							}
						}
						CAT_draw_queue_add(tile_mark_flip_sprite, 0, 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
					}
					else
					{
						CAT_draw_queue_add(cursor_flip_sprite, 0, 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
					}
					
					break;
				}
				case REMOVE:
				{
					if(deco_state.mod_idx != -1)
					{
						for(int y = deco_state.mod_rect.min.y; y <= deco_state.mod_rect.max.y; y++)
						{
							for(int x = deco_state.mod_rect.min.x; x <= deco_state.mod_rect.max.x; x++)
							{
								CAT_draw_queue_add(tile_hl_rm_sprite, 0, 3, x * 16, y * 16, CAT_DRAW_MODE_DEFAULT);
							}
						}
						CAT_draw_queue_add(tile_mark_rm_sprite, 0, 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
					}
					else
					{
						CAT_draw_queue_add(cursor_remove_sprite, 0, 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
					}
					break;
				}
			}
		}

		CAT_draw_queue_add(sbut_feed_sprite, 0, 3, 8, 280, CAT_DRAW_MODE_DEFAULT); 
		CAT_draw_queue_add(sbut_study_sprite, 0, 3, 56, 280, CAT_DRAW_MODE_DEFAULT); 
		CAT_draw_queue_add(sbut_play_sprite, 0, 3, 104, 280, CAT_DRAW_MODE_DEFAULT);
		CAT_draw_queue_add(sbut_deco_sprite, 0, 3, 152, 280, CAT_DRAW_MODE_DEFAULT);
		CAT_draw_queue_add(sbut_menu_sprite, 0, 3, 200, 280, CAT_DRAW_MODE_DEFAULT);
		CAT_draw_queue_add(sbut_hl_sprite, 0, 4, 8+48*room.selector, 280, CAT_DRAW_MODE_DEFAULT);

		if(input.touch.pressure)
			CAT_draw_queue_add(sbut_hl_sprite, 0, 4, input.touch.x, input.touch.y, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y);
		
		CAT_draw_queue_submit(cycle);
	}
	else if(machine == CAT_MS_menu)
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
		CAT_gui_text("MENU ");
		CAT_gui_image(fbut_a_sprite, 1);
		CAT_gui_image(icon_enter_sprite, 0);
		CAT_gui_image(fbut_b_sprite, 1);
		CAT_gui_image(icon_exit_sprite, 0);

		CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
		for(int i = 0; i < 4; i++)
		{
			CAT_gui_text("#");
			CAT_gui_text(menu_state.items[i]);

			if(i == menu_state.selector)
				CAT_gui_image(icon_pointer_sprite, 0);

			CAT_gui_line_break();
		}
	}
	else if(machine == CAT_MS_stats)
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
		CAT_gui_text("INSIGHTS ");
		CAT_gui_image(fbut_b_sprite, 1);
		CAT_gui_image(icon_exit_sprite, 0);

		CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
		CAT_gui_image(pet_idle_sprite, 0); 
		CAT_gui_line_break();

		CAT_gui_image(icon_vig_sprite, 0);
		CAT_gui_text("VIG ");
		for(int i = 1; i <= 12; i++)
		{
			if(i <= pet.vigour)
				CAT_gui_image(cell_vig_sprite, 0);
			else
				CAT_gui_image(cell_empty_sprite, 0);
		}
		CAT_gui_line_break();

		CAT_gui_image(icon_foc_sprite, 0);
		CAT_gui_text("FOC ");
		for(int i = 1; i <= 12; i++)
		{
			if(i <= pet.focus)
				CAT_gui_image(cell_foc_sprite, 0);
			else
				CAT_gui_image(cell_empty_sprite, 0);
		}
		CAT_gui_line_break();

		CAT_gui_image(icon_spi_sprite, 0);
		CAT_gui_text("SPI ");
		for(int i = 1; i <= 12; i++)
		{
			if(i <= pet.spirit)
				CAT_gui_image(cell_spi_sprite, 0);
			else
				CAT_gui_image(cell_empty_sprite, 0);
		}
		CAT_gui_line_break();

		int temp_idx = quantize(CAT_temp_score(), 1, 3);
		CAT_gui_image(icon_temp_sprite[temp_idx], 0);
		int co2_idx =  quantize(CAT_CO2_score(), 1, 3);
		CAT_gui_image(icon_co2_sprite[co2_idx], 0);
		int pm_idx =  quantize(CAT_PM_score(), 1, 3);
		CAT_gui_image(icon_pm_sprite[pm_idx], 0);
		int voc_idx =  quantize(CAT_VOC_score(), 1, 3);
		CAT_gui_image(icon_voc_sprite[voc_idx], 0);
		int nox_idx =  quantize(CAT_NOX_score(), 1, 3);
		CAT_gui_image(icon_nox_sprite[nox_idx], 0);
	}
	else if(machine == CAT_MS_bag)
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
		CAT_gui_text("BAG ");
		CAT_gui_image(fbut_a_sprite, 1);
		CAT_gui_image(icon_enter_sprite, 0);
		CAT_gui_image(fbut_b_sprite, 1);
		CAT_gui_image(icon_exit_sprite, 0);

		CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
		for(int i = 0; i < 9; i++)
		{
			int idx = bag_state.base + i;
			if(idx >= bag.length)
				return;

			int item_id = bag.item_id[idx];
			CAT_item* item = CAT_item_get(item_id);

			CAT_gui_panel_tight((CAT_ivec2) {0, 32+i*32}, (CAT_ivec2) {15, 2});
			switch(item->type)
			{
				case CAT_ITEM_TYPE_KEY:
					CAT_gui_image(icon_key_sprite, 0); 
					break;
				case CAT_ITEM_TYPE_FOOD:
					CAT_gui_image(icon_food_sprite, 0); 
					break;
				case CAT_ITEM_TYPE_PROP:
					CAT_gui_image(icon_prop_sprite, 0); 
					break;
			}

			char text[64];
			if(item->type == CAT_ITEM_TYPE_PROP)
				sprintf(text, " %s *%d", item->name, bag.count[idx]);
			else
				sprintf(text, " %s", item->name);
			if
			(
				(bag_state.destination == CAT_MS_deco && item->type != CAT_ITEM_TYPE_PROP) ||
				(bag_state.destination == CAT_MS_feed && item->type != CAT_ITEM_TYPE_FOOD)
			)
			{
				gui.text_mode = CAT_TEXT_MODE_STRIKETHROUGH;
			}	
			CAT_gui_text(text);
			gui.text_mode = CAT_TEXT_MODE_NORMAL;

			if(idx == bag_state.idx)
			{
				CAT_gui_image(icon_pointer_sprite, 0);
			}
		}
	}
	else if(machine == CAT_MS_manual)
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
		CAT_gui_text("CONTROLS ");
		CAT_gui_image(fbut_b_sprite, 1);
		CAT_gui_image(icon_exit_sprite, 0);

		CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
		CAT_gui_image(fbut_start_sprite, 0);
		CAT_gui_text("Open/close menu");
		CAT_gui_line_break();
		CAT_gui_image(fbut_select_sprite, 0);
		CAT_gui_text("Cycle decor mode");
		CAT_gui_line_break();
		CAT_gui_image(fbut_a_sprite, 0);
		CAT_gui_text("Confirm");
		CAT_gui_line_break();
		CAT_gui_image(fbut_b_sprite, 0);
		CAT_gui_text("Cancel");
		CAT_gui_line_break();
		CAT_gui_image(fbut_n_sprite, 0);
		CAT_gui_text("Navigate up");
		CAT_gui_line_break();
		CAT_gui_image(fbut_e_sprite, 0);
		CAT_gui_text("Navigate right");
		CAT_gui_line_break();
		CAT_gui_image(fbut_s_sprite, 0);
		CAT_gui_text("Navigate down");
		CAT_gui_line_break();
		CAT_gui_image(fbut_w_sprite, 0);
		CAT_gui_text("Navigate left");
	}
}

#pragma endregion

#pragma region MAIN

void CAT_init()
{
	CAT_rand_init();
	CAT_platform_init();
	CAT_input_init();

	CAT_atlas_init();
	CAT_sprite_mass_define();

	CAT_spriter_init();
	CAT_draw_queue_init();
	CAT_gui_init();
	
	CAT_item_table_init();
	CAT_item_mass_define();

	CAT_timetable_init();

	CAT_pet_init();
	CAT_room_init();
	CAT_feed_state_init();
	CAT_study_state_init();
	CAT_play_state_init();
	CAT_deco_state_init();
	CAT_menu_state_init();
	CAT_bag_state_init();
	
	machine = NULL;
	CAT_machine_transition(&machine, CAT_MS_default);
}

void CAT_tick_logic()
{
	CAT_platform_tick();
	CAT_AQI_tick();
	CAT_input_tick();

	CAT_machine_tick(&machine);
}

void CAT_tick_render(int cycle)
{
	CAT_render(cycle);
}

#ifdef CAT_DESKTOP
int main()
{
	CAT_init();

	while (CAT_get_battery_pct() > 0)
	{
		CAT_tick_logic();
		CAT_tick_render(0);
		CAT_LCD_post(spriter.framebuffer);
	}

	CAT_spriter_cleanup();
#ifndef CAT_BAKED_ASSETS
	CAT_atlas_cleanup();
#endif
	CAT_platform_cleanup();
	return 0;
}
#endif

#pragma endregion