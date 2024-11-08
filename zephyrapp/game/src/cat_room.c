#include "cat_room.h"
#include "cat_input.h"
#include "cat_item.h"
#include <stdio.h>

CAT_room room;
CAT_machine_state machine;

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

int CAT_room_find(int item_id)
{
	for(int i = 0; i < room.prop_count; i++)
	{
		if(room.props[i] == item_id)
			return i;
	}
	return -1;
}

bool CAT_room_fits(CAT_rect rect)
{
	if(!CAT_rect_contains(room.bounds, rect))
		return false;

	for(int i = 0; i < room.prop_count; i++)
	{
		CAT_item* prop = CAT_item_get(room.props[i]);
		CAT_ivec2 shape = prop->data.prop_data.shape;
		CAT_rect bounds = CAT_rect_place(room.places[i], shape);
		
		if(CAT_rect_overlaps(rect, bounds))
			return false;
	}

	return true;
}

void CAT_room_place(int item_id, CAT_ivec2 place)
{
	int idx = room.prop_count;
	room.prop_count += 1;
	room.props[idx] = item_id;
	room.places[idx] = place;
	room.overrides[idx] = 0;
}

void CAT_room_remove(int idx)
{
	for(int i = idx; i < room.prop_count-1; i++)
	{
		room.props[i] = room.props[i+1];
		room.places[i] = room.places[i+1];
	}
	room.prop_count -= 1;
}

void CAT_room_flip(int idx)
{
	int item_id = room.props[idx];
	CAT_item* item = CAT_item_get(item_id);

	if(item->data.prop_data.animate)
	{
		room.overrides[idx] = !room.overrides[idx];
	}
	else
	{
		room.overrides[idx] += 1;
		int sprite_id = item->sprite_id;
		CAT_sprite* sprite = &atlas.table[sprite_id];
		if(room.overrides[idx] >= sprite->frame_count)
			room.overrides[idx] = 0;
	}
}

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
	room.cursor.x = min(max(room.cursor.x, room.bounds.min.x), room.bounds.max.x-1);
	room.cursor.y = min(max(room.cursor.y, room.bounds.min.y), room.bounds.max.y-1);
}

CAT_pet pet;

CAT_ASM_state* pet_asm;
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

CAT_ASM_state* bubl_asm;
CAT_ASM_state AS_react;

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

void CAT_MS_room(CAT_machine_signal signal)
{
	static CAT_vec2 poi;

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
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
			{
				room.selector += 1;
				if(room.selector > 4)
					room.selector = 0;
			}
			if(CAT_input_pulse(CAT_BUTTON_LEFT))
			{
				room.selector -= 1;
				if(room.selector < 0)
					room.selector = 4;
			}
			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_machine_transition(&machine, room.buttons[room.selector]);

			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_menu);

			if(CAT_input_touch(pet.pos.x, pet.pos.y-16, 16))
			{
				CAT_ASM_transition(&bubl_asm, &AS_react);
			}
			if(CAT_ASM_is_in(&bubl_asm, &AS_react))
			{
				if(CAT_timer_tick(pet.react_timer_id))
				{
					CAT_ASM_transition(&bubl_asm, NULL);
					CAT_timer_reset(pet.react_timer_id);
				}
			}

			if(CAT_timer_tick(pet.stat_timer_id))
			{
				CAT_pet_stat();
				CAT_timer_reset(pet.stat_timer_id);
			}

			if(CAT_ASM_is_in(&pet_asm, &AS_idle) && CAT_ASM_is_ticking(&pet_asm))
			{
				if(CAT_timer_tick(pet.walk_timer_id))
				{
					CAT_ivec2 grid_min = CAT_ivec2_add(room.bounds.min, (CAT_ivec2){1, 1});
					CAT_ivec2 grid_max = CAT_ivec2_add(room.bounds.max, (CAT_ivec2){-1, -1});
					CAT_vec2 world_min = CAT_iv2v(CAT_ivec2_mul(grid_min, 16));
					CAT_vec2 world_max = CAT_iv2v(CAT_ivec2_mul(grid_max, 16));
					poi = CAT_rand_vec2(world_min, world_max);

					CAT_ASM_transition(&pet_asm, &AS_walk);
					CAT_timer_reset(pet.walk_timer_id);
				}
			}
			if(CAT_ASM_is_in(&pet_asm, &AS_walk) && CAT_ASM_is_ticking(&pet_asm))
			{
				if(CAT_pet_seek(poi))
				{
					CAT_ASM_transition(&pet_asm, &AS_idle);
				}
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_render_room()
{
	CAT_draw_tiles(base_wall_sprite, 0, 0, 4);
	CAT_draw_tiles(base_wall_sprite, 1, 4, 1);
	CAT_draw_tiles(base_wall_sprite, 2, 5, 1);
	CAT_draw_tiles(base_floor_sprite, 0, 6, 1);
	CAT_draw_tiles(base_floor_sprite, 1, 7, 10);
	CAT_draw_tiles(base_floor_sprite, 2, 17, 3);

	CAT_draw_queue_add(window_day_sprite, 0, 2, 8, 8, CAT_DRAW_MODE_DEFAULT);
	CAT_draw_queue_add(vending_sprite, -1, 2, 164, 112, CAT_DRAW_MODE_BOTTOM);

	for(int i = 0; i < room.prop_count; i++)
	{
		int prop_id = room.props[i];
		CAT_item* prop = CAT_item_get(prop_id);
		CAT_ivec2 shape = prop->data.prop_data.shape;
		CAT_ivec2 place = room.places[i];

		int prop_mode = CAT_DRAW_MODE_BOTTOM;
		if(prop->data.prop_data.animate)
		{	
			if(room.overrides[i])
				prop_mode |= CAT_DRAW_MODE_REFLECT_X;
			CAT_draw_queue_animate(prop->sprite_id, 2, place.x * 16, (place.y+shape.y) * 16, prop_mode);
		}
		else
		{
			int frame_idx = room.overrides[i];
			CAT_draw_queue_add(prop->sprite_id, frame_idx, 2, place.x * 16, (place.y+shape.y) * 16, prop_mode);
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

	CAT_draw_queue_add(sbut_feed_sprite, 0, 3, 8, 280, CAT_DRAW_MODE_DEFAULT); 
	CAT_draw_queue_add(sbut_study_sprite, 0, 3, 56, 280, CAT_DRAW_MODE_DEFAULT); 
	CAT_draw_queue_add(sbut_play_sprite, 0, 3, 104, 280, CAT_DRAW_MODE_DEFAULT);
	CAT_draw_queue_add(sbut_deco_sprite, 0, 3, 152, 280, CAT_DRAW_MODE_DEFAULT);
	CAT_draw_queue_add(sbut_menu_sprite, 0, 3, 200, 280, CAT_DRAW_MODE_DEFAULT);
	CAT_draw_queue_add(sbut_hl_sprite, 0, 4, 8+48*room.selector, 280, CAT_DRAW_MODE_DEFAULT);

	if(input.touch.pressure)
		CAT_draw_queue_add(sbut_hl_sprite, 0, 4, input.touch.x, input.touch.y, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y);
}
