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

CAT_machine_state machine;
void CAT_MS_default(CAT_machine_signal);
void CAT_MS_feed(CAT_machine_signal);
void CAT_MS_deco(CAT_machine_signal);
void CAT_MS_menu(CAT_machine_signal);
void CAT_MS_stats(CAT_machine_signal);
void CAT_MS_bag(CAT_machine_signal);

typedef struct CAT_pet
{
	int sprite_id;

	float vigour;
	float focus;
	float spirit;
	float delta_vigour;
	float delta_focus;
	float delta_spirit;

	CAT_vec2 targ;
	CAT_vec2 pos;
	CAT_vec2 vel;
	
	int walk_timer_id;
	int mood_timer_id;
	int stat_timer_id;
} CAT_pet;
CAT_pet pet;

typedef struct CAT_room
{
	CAT_rect bounds;
	CAT_ivec2 cursor;

	CAT_machine_state buttons[5];
	int selector;
} CAT_room;
CAT_room room;

typedef struct CAT_feed_state
{
	bool confirmed;
	bool found;
	int food_id;
} CAT_feed_state;
CAT_feed_state feed_state;

typedef struct CAT_deco_state
{
	int props[CAT_MAX_PROP_COUNT];
	CAT_ivec2 places[CAT_MAX_PROP_COUNT];
	int prop_count;

	enum mode {ADD, REMOVE} mode;

	int add_id;
	CAT_rect add_rect;
	bool valid_add;

	int remove_id;
	CAT_rect remove_rect;
} CAT_deco_state;
CAT_deco_state deco_state;

typedef struct CAT_menu_state
{
	const char* items[3];
	int selector;
} CAT_menu_state;
CAT_menu_state menu_state;

typedef struct CAT_bag_state
{
	int base;
	int visible[9];
	int seen;
	int selector;
	CAT_machine_state destination;
} CAT_bag_state;
CAT_bag_state bag_state;

#pragma endregion

#pragma region PET

void CAT_pet_stat()
{
	pet.vigour += pet.delta_vigour;
	pet.focus += pet.delta_focus;
	pet.spirit += pet.delta_spirit;
}

bool CAT_pet_seek()
{
	CAT_vec2 line = CAT_vec2_sub(pet.targ, pet.pos);
	float dist = CAT_vec2_mag(line);
	if(dist < 8)
	{
		pet.vel = (CAT_vec2) {0, 0};
		return true;
	}
	else
	{
		pet.vel = CAT_vec2_mul(line, 48.0f/dist);
		pet.pos = CAT_vec2_add(pet.pos, CAT_vec2_mul(pet.vel, CAT_get_delta_time()));
		return false;
	}
}

void CAT_pet_eat(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	pet.vigour += item->data.food_data.d_v;
	pet.focus += item->data.food_data.d_f;
	pet.spirit += item->data.food_data.d_s;
	pet.delta_vigour += item->data.food_data.dd_v;
	pet.delta_focus += item->data.food_data.dd_f;
	pet.delta_spirit += item->data.food_data.dd_s;
}

void CAT_pet_transition(int sprite_id)
{
	CAT_anim_reset(pet.sprite_id);
	pet.sprite_id = sprite_id;
}

void CAT_pet_init()
{
	pet.sprite_id = pet_idle_sprite;

	pet.vigour = 12;
	pet.focus = 12;
	pet.spirit = 12;
	pet.delta_vigour = -1;
	pet.delta_focus = -1;
	pet.delta_spirit = -1;

	pet.targ = (CAT_vec2) {-1, -1};
	pet.vel = (CAT_vec2) {0, 0};
	pet.pos = (CAT_vec2) {120, 200};
	
	pet.walk_timer_id = CAT_timer_init(3.0f);
	pet.mood_timer_id = CAT_timer_init(2.0f);
	pet.stat_timer_id = CAT_timer_init(0.25f);
}

#pragma endregion

#pragma region ROOM

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
	room.buttons[1] = CAT_MS_feed;
	room.buttons[2] = CAT_MS_feed;
	room.buttons[3] = CAT_MS_deco;
	room.buttons[4] = CAT_MS_menu;
	room.selector = 0;
}

#pragma region DEFAULT

void CAT_MS_default(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_pet_transition(pet_idle_sprite);
			CAT_timer_reset(pet.walk_timer_id);
			CAT_timer_reset(pet.mood_timer_id);
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

			if(pet.sprite_id == pet_idle_sprite)
			{
				if(CAT_timer_tick(pet.walk_timer_id))
				{
					CAT_vec2 world_min = CAT_iv2v(CAT_ivec2_mul(room.bounds.min, 16));
					CAT_vec2 world_max = CAT_iv2v(CAT_ivec2_mul(room.bounds.max, 16));
					pet.targ = CAT_rand_vec2(world_min, world_max);
					CAT_pet_transition(pet_walk_sprite);
				}
			}
			else if(pet.sprite_id == pet_walk_sprite)
			{
				if(CAT_pet_seek())
					CAT_pet_transition(pet_idle_sprite);
			}

			if(CAT_timer_tick(pet.stat_timer_id))
				CAT_pet_stat();
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

#pragma endregion

#pragma region FEED

void CAT_feed_state_init()
{
	feed_state.confirmed = false;
	feed_state.found = false;
	feed_state.food_id = -1;
}

void CAT_MS_feed(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_pet_transition(pet_idle_sprite);
			feed_state.confirmed = false;
			feed_state.found = false;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(feed_state.food_id == -1)
			{
				if(CAT_input_pressed(CAT_BUTTON_A))
				{
					bag_state.destination = CAT_MS_feed;
					CAT_machine_transition(&machine, CAT_MS_bag);
				}
			}
				
			if(!feed_state.confirmed)
			{
				CAT_room_move_cursor();

				if(CAT_input_pressed(CAT_BUTTON_A))
				{
					int cx_real = room.cursor.x * 16;
					int x_off = cx_real >= pet.pos.x ? -8 : 32;
					pet.targ = (CAT_vec2) {cx_real + x_off, room.cursor.y * 16 + 24};
					feed_state.confirmed = true;
					CAT_pet_transition(pet_walk_sprite);
				}
			}
			else if(!feed_state.found)
			{
				if(CAT_pet_seek())
				{
					feed_state.found = true;
					CAT_pet_transition(pet_eat_sprite);	
				}
			}

			if(pet.sprite_id == pet_eat_sprite)
			{
				if(CAT_anim_finished(pet_eat_sprite))
				{
					CAT_timer_reset(pet.mood_timer_id);
					CAT_pet_transition(pet_chew_sprite);
				}
			}
			else if(pet.sprite_id == pet_chew_sprite)
			{
				if(CAT_timer_tick(pet.mood_timer_id))
				{
					CAT_pet_eat(feed_state.food_id);
					CAT_machine_transition(&machine, CAT_MS_default);
				}
			}
			
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_default);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			feed_state.food_id = -1;
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
	deco_state.remove_id = -1;
}

void CAT_MS_deco(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_pet_transition(pet_idle_sprite);
			deco_state.remove_id = -1;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_room_move_cursor();

			if(CAT_input_pressed(CAT_BUTTON_SELECT))
			{
				if(deco_state.mode == ADD)
					deco_state.mode = REMOVE;
				else
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
						deco_state.add_rect = bounds;

						deco_state.valid_add = true;
						if(!CAT_test_contains(room.bounds, bounds))
							deco_state.valid_add = false;
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
				case REMOVE:
				{
					if(deco_state.remove_id != -1)
					{
						if(!CAT_test_pt_rect(room.cursor, deco_state.remove_rect))
						{
							deco_state.remove_id = -1;
						}
						else if(CAT_input_pressed(CAT_BUTTON_A))
						{
							CAT_bag_add(deco_state.props[deco_state.remove_id]);
							for(int i = deco_state.remove_id; i < deco_state.prop_count-1; i++)
							{
								deco_state.props[i] = deco_state.props[i+1];
								deco_state.places[i] = deco_state.places[i+1];
							}
							deco_state.prop_count -= 1;
							deco_state.remove_id = -1;
						}
					}
					else
					{
						for(int i = 0; i < deco_state.prop_count; i++)
						{	
							CAT_item* prop = &item_table.data[deco_state.props[i]];
							CAT_ivec2 shape = prop->data.prop_data.shape;
							CAT_rect bounds = CAT_rect_place(deco_state.places[i], shape);
							bounds.max.x -= 1;
							bounds.max.y -= 1;

							if(CAT_test_pt_rect(room.cursor, bounds))
							{
								deco_state.remove_id = i;
								deco_state.remove_rect = bounds;
							}
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
	menu_state.items[0] = "STATS";
	menu_state.items[1] = "BAG";
	menu_state.items[2] = "BACK";
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
			menu_state.selector = clamp(menu_state.selector, 0, 2);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(menu_state.selector == 0)
					CAT_machine_transition(&machine, CAT_MS_stats);
				if(menu_state.selector == 1)
					CAT_machine_transition(&machine, CAT_MS_bag);
				if(menu_state.selector == 2)
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
	bag_state.seen = 0;
	bag_state.selector = 0;
	bag_state.destination = CAT_MS_default;
}

void CAT_bag_state_refresh()
{
	bag_state.seen = 0;
	for(int i = bag_state.base; bag_state.seen < 9 && i < item_table.length; i++)
	{
		CAT_item* item = &item_table.data[i];
		if
		(
			item->count > 0 &&
			(
				bag_state.destination == CAT_MS_default ||
				(bag_state.destination == CAT_MS_feed && item->type == CAT_ITEM_TYPE_FOOD) ||
				(bag_state.destination == CAT_MS_deco && item->type == CAT_ITEM_TYPE_PROP)
			)
		)
		{
			bag_state.visible[bag_state.seen] = i;
			bag_state.seen += 1;
		}
	}
	bag_state.selector = clamp(bag_state.selector, 0, bag_state.seen-1);
}

void CAT_MS_bag(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			bag_state.base = 0;
			bag_state.seen = 0;
			bag_state.selector = 0;
			CAT_bag_state_refresh();
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pulse(CAT_BUTTON_UP))
			{
				bag_state.selector -= 1;
				if(bag_state.selector < 0)
				{
					int prev = CAT_bag_prev(bag_state.base);
					if(prev != -1)
					{
						bag_state.base = prev;
						CAT_bag_state_refresh();
					}
					bag_state.selector = 0;
				}
			}
				
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
			{
				bag_state.selector += 1;
				if(bag_state.selector >= 9)
				{
					int next = CAT_bag_next(bag_state.visible[bag_state.seen-1]);
					if(next != -1)
					{
						bag_state.base = CAT_bag_next(bag_state.base);
					}
					bag_state.selector = 8;
				}
				CAT_bag_state_refresh();
			}

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				int item_id = bag_state.visible[bag_state.selector];
				CAT_item* item = &item_table.data[item_id];

				if(item->type == CAT_ITEM_TYPE_FOOD && (bag_state.destination == CAT_MS_default || bag_state.destination == CAT_MS_feed))
				{
					feed_state.food_id = item_id;
					CAT_machine_transition(&machine, CAT_MS_feed);
				}
				else if(item->type == CAT_ITEM_TYPE_PROP && (bag_state.destination == CAT_MS_default || bag_state.destination == CAT_MS_deco))
				{
					deco_state.add_id = item_id;
					CAT_machine_transition(&machine, CAT_MS_deco);
				}
			}
				
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, bag_state.destination);
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_default);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			bag_state.destination = CAT_MS_default;
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
		machine == CAT_MS_deco
	)
	{
		CAT_draw_tiles(base_wall_sprite, 0, 0, 4);
		CAT_draw_tiles(base_wall_sprite, 1, 4, 1);
		CAT_draw_tiles(base_wall_sprite, 2, 5, 1);
		CAT_draw_tiles(base_floor_sprite, 0, 6, 1);
		CAT_draw_tiles(base_floor_sprite, 1, 7, 10);
		CAT_draw_tiles(base_floor_sprite, 2, 17, 3);

		CAT_draw_queue_add(window_day_sprite, 0, 2, 16, 8, CAT_DRAW_MODE_DEFAULT);
		CAT_draw_queue_add_anim(vending_sprite, 2, 164, 112, CAT_DRAW_MODE_BOTTOM);
		
		for(int i = 0; i < deco_state.prop_count; i++)
		{
			int prop_id = deco_state.props[i];
			CAT_item* prop = CAT_item_get(prop_id);
			CAT_ivec2 shape = prop->data.prop_data.shape;
			CAT_ivec2 place = deco_state.places[i];
			CAT_draw_queue_add_anim(prop->sprite_id, 2, place.x * 16, (place.y+shape.y) * 16, CAT_DRAW_MODE_BOTTOM);
		}

		int pet_mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;
		pet_mode |= (pet.pos.x < pet.targ.x ? CAT_DRAW_MODE_REFLECT_X : 0);
		CAT_draw_queue_add_anim(pet.sprite_id, 2, pet.pos.x, pet.pos.y, pet_mode);	

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
		else if(machine == CAT_MS_deco)
		{
			switch(deco_state.mode)
			{
				case ADD:
				{
					if(deco_state.add_id != -1)
					{
						for(int y = deco_state.add_rect.min.y; y < deco_state.add_rect.max.y; y++)
						{
							for(int x = deco_state.add_rect.min.x; x < deco_state.add_rect.max.x; x++)
							{
								CAT_draw_queue_add(tile_hl_add_sprite, 0, 3, x * 16, y * 16, CAT_DRAW_MODE_DEFAULT);
							}
						}
					}
					else
					{
						CAT_draw_queue_add(cursor_add_sprite, 0, 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
					}
					break;
				}
				case REMOVE:
				{
					if(deco_state.remove_id != -1)
					{
						for(int y = deco_state.remove_rect.min.y; y <= deco_state.remove_rect.max.y; y++)
						{
							for(int x = deco_state.remove_rect.min.x; x <= deco_state.remove_rect.max.x; x++)
							{
								CAT_draw_queue_add(tile_hl_rm_outer_sprite, 0, 3, x * 16, y * 16, CAT_DRAW_MODE_DEFAULT);
							}
						}
						CAT_draw_queue_add(tile_hl_rm_inner_sprite, 0, 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
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
		if(machine == CAT_MS_default)
			CAT_draw_queue_add(sbut_hl_sprite, 0, 4, 8+48*room.selector, 280, CAT_DRAW_MODE_DEFAULT);

		if(input.touch.pressure)
			CAT_draw_queue_add(sbut_hl_sprite, 0, 4, input.touch.x, input.touch.y, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y);
		
		CAT_draw_queue_submit(cycle);
	}
	else if(machine == CAT_MS_menu)
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
		CAT_gui_text("MENU");
		CAT_gui_image(fbut_a_sprite, 0);
		CAT_gui_image(icon_enter_sprite, 0);
		CAT_gui_image(fbut_b_sprite, 0);
		CAT_gui_image(icon_exit_sprite, 0);

		CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
		for(int i = 0; i < 3; i++)
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
		CAT_gui_text("STATS");
		CAT_gui_image(fbut_b_sprite, 0);
		CAT_gui_image(icon_exit_sprite, 0);

		CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
		CAT_gui_image(pet_idle_sprite, 0); 
		CAT_gui_line_break();

		CAT_gui_image(icon_vig_sprite, 0);
		CAT_gui_text("VIG");
		for(int i = 1; i <= 12; i++)
		{
			if(i <= pet.vigour)
				CAT_gui_image(cell_vig_sprite, 0);
			else
				CAT_gui_image(cell_empty_sprite, 0);
		}
		CAT_gui_line_break();

		CAT_gui_image(icon_foc_sprite, 0);
		CAT_gui_text("FOC");
		for(int i = 1; i <= 12; i++)
		{
			if(i <= pet.focus)
				CAT_gui_image(cell_foc_sprite, 0);
			else
				CAT_gui_image(cell_empty_sprite, 0);
		}
		CAT_gui_line_break();

		CAT_gui_image(icon_spi_sprite, 0);
		CAT_gui_text("SPI");
		for(int i = 1; i <= 12; i++)
		{
			if(i <= pet.spirit)
				CAT_gui_image(cell_spi_sprite, 0);
			else
				CAT_gui_image(cell_empty_sprite, 0);
		}
		CAT_gui_line_break();

		CAT_gui_image(crisis_co2_sprite, 0);
		CAT_gui_image(crisis_nox_sprite, 0);
		CAT_gui_image(crisis_vol_sprite, 0);
		CAT_gui_line_break();
		CAT_gui_image(crisis_hot_sprite, 0);
		CAT_gui_image(crisis_cold_sprite, 0);

	}
	else if(machine == CAT_MS_bag)
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
		CAT_gui_text("BAG");
		CAT_gui_image(fbut_b_sprite, 0);
		CAT_gui_image(icon_exit_sprite, 0);

		CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
		for(int i = 0; i < bag_state.seen; i++)
		{
			int item_id = bag_state.visible[i];
			CAT_item* item = CAT_item_get(item_id);

			CAT_gui_panel((CAT_ivec2) {0, 32+i*32}, (CAT_ivec2) {15, 2});
			CAT_gui_image(icon_food_sprite, 0); 
			char text[64];
			sprintf(text, "%s *%d", item->name, item->count);
			CAT_gui_text(text);

			if(i == bag_state.selector)
			{
				CAT_gui_image(icon_pointer_sprite, 0);
			}
		}
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
	
	CAT_gui_init(panel_sprite, glyph_sprite);
	
	CAT_item_table_init();
	CAT_item_mass_define();

	CAT_timetable_init();

	CAT_pet_init();
	CAT_room_init();
	CAT_feed_state_init();
	CAT_deco_state_init();
	CAT_menu_state_init();
	CAT_bag_state_init();
	
	machine = NULL;
	CAT_machine_transition(&machine, CAT_MS_default);
}

void CAT_tick_logic()
{
	CAT_platform_tick();
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