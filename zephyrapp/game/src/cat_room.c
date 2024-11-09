#include "cat_room.h"
#include "cat_input.h"
#include "cat_item.h"
#include <stdio.h>
#include "cat_pet.h"
#include "cat_sprite.h"
#include "cat_gui.h"
#include "cat_bag.h"

CAT_room room;
CAT_vec2 poi;

void CAT_room_init()
{
	room.bounds = (CAT_rect){{0, 7}, {15, 17}};
	room.cursor = room.bounds.min;

	room.prop_count = 0;

	room.coin_count = 0;
	room.crypto_timer_id = CAT_timer_init(5.0f);

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
		CAT_rect bounds = CAT_rect_place(room.prop_places[i], shape);
		
		if(CAT_rect_overlaps(rect, bounds))
			return false;
	}

	return true;
}

void CAT_room_add_prop(int item_id, CAT_ivec2 place)
{
	int idx = room.prop_count;
	room.prop_count += 1;
	room.props[idx] = item_id;
	room.prop_places[idx] = place;
	room.prop_overrides[idx] = 0;
}

void CAT_room_remove_prop(int idx)
{
	for(int i = idx; i < room.prop_count-1; i++)
	{
		room.props[i] = room.props[i+1];
		room.prop_places[i] = room.prop_places[i+1];
	}
	room.prop_count -= 1;
}

void CAT_room_flip_prop(int idx)
{
	int item_id = room.props[idx];
	CAT_item* item = CAT_item_get(item_id);
	CAT_sprite* sprite = &atlas.table[item->sprite_id];

	if(item->data.prop_data.animate || sprite->frame_count == 1)
	{
		room.prop_overrides[idx] = !room.prop_overrides[idx];
	}
	else
	{
		room.prop_overrides[idx] += 1;
		if(room.prop_overrides[idx] >= sprite->frame_count)
			room.prop_overrides[idx] = 0;
	}
}

void CAT_room_add_coin(CAT_vec2 origin, CAT_vec2 place)
{
	int idx = room.coin_count;
	room.coin_count += 1;
	room.coin_origins[idx] = origin;
	room.coin_places[idx] = place;
	room.coin_timers[idx] = CAT_timer_init(0.75f);
}

void CAT_room_remove_coin(int idx)
{
	for(int i = idx; i < room.coin_count-1; i++)
	{
		room.coin_places[i] = room.coin_places[i+1];
		room.coin_timers[i] = room.coin_timers[i+1];
	}
	room.coin_count -= 1;
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

void CAT_MS_room(CAT_machine_signal signal)
{
	

	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_AM_transition(&pet_asm, &AS_idle);
			CAT_AM_transition(&bubble_asm, NULL);
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
				CAT_AM_transition(&bubble_asm, &AS_react);
			}
			if(CAT_AM_is_in(&bubble_asm, &AS_react))
			{
				if(CAT_timer_tick(pet.react_timer_id))
				{
					CAT_AM_transition(&bubble_asm, NULL);
					CAT_timer_reset(pet.react_timer_id);
				}
			}

			if(CAT_timer_tick(pet.stat_timer_id))
			{
				CAT_pet_stat();
				CAT_timer_reset(pet.stat_timer_id);
			}

			if(CAT_AM_is_in(&pet_asm, &AS_idle) && CAT_AM_is_ticking(&pet_asm))
			{
				if(CAT_timer_tick(pet.walk_timer_id))
				{
					CAT_ivec2 grid_min = CAT_ivec2_add(room.bounds.min, (CAT_ivec2){1, 1});
					CAT_ivec2 grid_max = CAT_ivec2_add(room.bounds.max, (CAT_ivec2){-1, -1});
					CAT_vec2 world_min = CAT_iv2v(CAT_ivec2_mul(grid_min, 16));
					CAT_vec2 world_max = CAT_iv2v(CAT_ivec2_mul(grid_max, 16));
					poi = CAT_rand_vec2(world_min, world_max);

					CAT_AM_transition(&pet_asm, &AS_walk);
					CAT_timer_reset(pet.walk_timer_id);
				}
			}
			if(CAT_AM_is_in(&pet_asm, &AS_walk) && CAT_AM_is_ticking(&pet_asm))
			{
				if(CAT_pet_seek(poi))
				{
					CAT_AM_transition(&pet_asm, &AS_idle);
				}
			}

			if(CAT_timer_tick(room.crypto_timer_id) && room.coin_count < CAT_MAX_COIN_COUNT)
			{
				for(int i = 0; i < room.prop_count; i++)
				{
					if(room.props[i] == crypto_item)
					{
						float xi = room.prop_places[i].x * 16 + 24;
						float yi = room.prop_places[i].y * 16 - 24.0f;
						float xf = CAT_rand_float((room.bounds.min.x+1) * 16, (room.bounds.max.x-1) * 16);
						float yf = CAT_rand_float((room.bounds.min.y+1) * 16, (room.bounds.max.y-1) * 16);
						printf("%f %f %f %f\n", xi, yi, xf, yf);
						CAT_room_add_coin((CAT_vec2) {xi, yi}, (CAT_vec2) {xf, yf});
					}
				}
				CAT_timer_reset(room.crypto_timer_id);
			}
			for(int i = 0; i < room.coin_count; i++)
			{
				if(CAT_timer_tick(room.coin_timers[i]))
				{
					CAT_vec2 place = room.coin_places[i];
					if(CAT_input_touch(place.x, place.y - 16, 16))
					{
						bag.coins += 1;
						CAT_room_remove_coin(i);
						i -= 1;
					}
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

void CAT_render_room(int cycle)
{
	CAT_draw_tiles(base_wall_sprite, 0, 0, 4);
	CAT_draw_tiles(base_wall_sprite, 1, 4, 1);
	CAT_draw_tiles(base_wall_sprite, 2, 5, 1);
	CAT_draw_tiles(base_floor_sprite, 0, 6, 1);
	CAT_draw_tiles(base_floor_sprite, 1, 7, 10);
	CAT_draw_tiles(base_floor_sprite, 2, 17, 3);

	if (cycle == 0)
	{
		CAT_draw_queue_add(window_day_sprite, 0, 2, 8, 8, CAT_DRAW_MODE_DEFAULT);
		CAT_draw_queue_add(vending_sprite, -1, 2, 164, 112, CAT_DRAW_MODE_BOTTOM);

		for(int i = 0; i < room.prop_count; i++)
		{
			int prop_id = room.props[i];
			CAT_item* prop = CAT_item_get(prop_id);
			CAT_ivec2 shape = prop->data.prop_data.shape;
			CAT_ivec2 place = room.prop_places[i];

			int prop_mode = CAT_DRAW_MODE_BOTTOM;
			if(prop->data.prop_data.animate)
			{	
				if(room.prop_overrides[i])
					prop_mode |= CAT_DRAW_MODE_REFLECT_X;
				CAT_draw_queue_animate(prop->sprite_id, 2, place.x * 16, (place.y+shape.y) * 16, prop_mode);
			}
			else if(atlas.table[prop->sprite_id].frame_count == 1)
			{
				if(room.prop_overrides[i])
					prop_mode |= CAT_DRAW_MODE_REFLECT_X;
				CAT_draw_queue_add(prop->sprite_id, 0, 2, place.x * 16, (place.y+shape.y) * 16, prop_mode);
			}
			else
			{
				int frame_idx = room.prop_overrides[i];
				CAT_draw_queue_add(prop->sprite_id, frame_idx, 2, place.x * 16, (place.y+shape.y) * 16, prop_mode);
			}
		}

		for(int i = 0; i < room.coin_count; i++)
		{
			CAT_vec2 origin = room.coin_origins[i];
			CAT_vec2 place = room.coin_places[i];

			float t = CAT_timer_progress(room.coin_timers[i]);
			float x = lerp(origin.x, place.x, t);
			float y = lerp(origin.y, place.y, t);

			CAT_draw_queue_add(icon_coin_sprite, 0, 2, x, y, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_BOTTOM);
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
}
