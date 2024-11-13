#include "cat_room.h"

#include "cat_input.h"
#include "cat_item.h"
#include <stdio.h>
#include "cat_pet.h"
#include "cat_sprite.h"
#include "cat_gui.h"
#include "cat_bag.h"

CAT_room room =
{
	.bounds = {{0, 7}, {15, 17}},
	.cursor = {7, 12},

	.prop_count = 0,

	.coin_count = 0,
	.earn_timer_id = -1,

	.buttons =
	{
		CAT_MS_feed,
		CAT_MS_study,
		CAT_MS_play,
		CAT_MS_deco,
		CAT_MS_menu
	},
	.selector = 0
};

CAT_vec2 poi = (CAT_vec2){120, 200};

int CAT_room_find(int item_id)
{
	for(int i = 0; i < room.prop_count; i++)
	{
		if(room.prop_ids[i] == item_id)
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
		CAT_item* prop = CAT_item_get(room.prop_ids[i]);
		CAT_ivec2 shape = prop->data.prop_data.shape;
		CAT_rect bounds = CAT_rect_place(room.prop_places[i], shape);
		
		if(CAT_rect_overlaps(rect, bounds))
			return false;
	}

	return true;
}

void CAT_room_add_prop(int item_id, CAT_ivec2 place)
{
	if(room.prop_count >= CAT_MAX_PROP_COUNT)
		return;

	int idx = room.prop_count;
	room.prop_count += 1;
	room.prop_ids[idx] = item_id;
	room.prop_places[idx] = place;
	room.prop_overrides[idx] = 0;
}

void CAT_room_remove_prop(int idx)
{
	if(idx < 0 || idx >= room.prop_count)
		return;

	for(int i = idx; i < room.prop_count-1; i++)
	{
		room.prop_ids[i] = room.prop_ids[i+1];
		room.prop_places[i] = room.prop_places[i+1];
	}
	room.prop_count -= 1;
}

void CAT_room_flip_prop(int idx)
{
	if(idx < 0 || idx >= room.prop_count)
		return;

	int item_id = room.prop_ids[idx];
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
	if(room.coin_count >= CAT_MAX_COIN_COUNT)
		return;

	int idx = room.coin_count;
	room.coin_count += 1;
	room.coin_origins[idx] = origin;
	room.coin_places[idx] = place;
	CAT_timer_reset(room.coin_move_timers[idx]);
}

void CAT_room_remove_coin(int idx)
{
	if(idx < 0 || idx >= room.coin_count)
		return;
	
	int old_timer = room.coin_move_timers[idx];
	for(int i = idx; i < room.coin_count-1; i++)
	{
		room.coin_places[i] = room.coin_places[i+1];
		room.coin_origins[i] = room.coin_origins[i+1];
		room.coin_move_timers[i] = room.coin_move_timers[i+1];
	}
	room.coin_count -= 1;
	room.coin_move_timers[room.coin_count] = old_timer;
}

void CAT_room_earn(int ticks)
{
	for(int i = 0; i < room.prop_count; i++)
	{
		if(room.prop_ids[i] == gpu_item)
		{
			for(int t = 0; t < ticks; t++)
			{
				float xi = room.prop_places[i].x * 16 + 24;
				float yi = room.prop_places[i].y * 16 - 24.0f;
				float xf = CAT_rand_float((room.bounds.min.x+1) * 16, (room.bounds.max.x-1) * 16);
				float yf = CAT_rand_float((room.bounds.min.y+1) * 16, (room.bounds.max.y-1) * 16);
				CAT_room_add_coin((CAT_vec2) {xi, yi}, (CAT_vec2) {xf, yf});
			}
		}
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

void CAT_room_init()
{
	for(int i = 0; i < CAT_MAX_COIN_COUNT; i++)
	{
		room.coin_move_timers[i] = CAT_timer_init(0.75f);
	}
	room.earn_timer_id = CAT_timer_init(CAT_COIN_TICK_SECS);
}

void CAT_room_background_tick(bool capture_input)
{
	if(CAT_timer_tick(room.earn_timer_id))
	{
		CAT_room_earn(1);
		CAT_timer_reset(room.earn_timer_id);
	}

	if(!capture_input)
		return;

	for(int i = 0; i < 5; i++)
	{
		if(CAT_input_touch(8+16+48*i, 280+16, 16))
		{
			CAT_machine_transition(room.buttons[i]);
			room.selector = i;
		}
	}

	if(CAT_input_touch_rect(176, 16, 56, 96))
		CAT_machine_transition(CAT_MS_vending);
	if(CAT_input_touch_rect(128, 48, 32, 64))
		CAT_machine_transition(CAT_MS_arcade);

	for(int i = 0; i < room.coin_count; i++)
	{
		if(CAT_timer_tick(room.coin_move_timers[i]))
		{
			CAT_vec2 place = room.coin_places[i];
			if(CAT_input_drag(place.x + 8, place.y - 8, 16))
			{
				bag.coins += 1;
				CAT_room_remove_coin(i);
				i -= 1;
			}
		}
	}
}

void CAT_MS_room(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_pet_settle();
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_menu);

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
				CAT_machine_transition(room.buttons[room.selector]);	
			
			if(!CAT_pet_is_critical())
			{
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
			}
			else
			{
				if(!CAT_AM_is_in(&pet_asm, &AS_crit))
					CAT_AM_transition(&pet_asm, &AS_crit);
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
		CAT_draw_queue_add(vending_sprite, -1, 2, 172, 16, CAT_DRAW_MODE_DEFAULT);
		CAT_draw_queue_add(arcade_sprite, -1, 2, 124, 48, CAT_DRAW_MODE_DEFAULT);

		for(int i = 0; i < room.prop_count; i++)
		{
			int prop_id = room.prop_ids[i];
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

			float t = CAT_timer_progress(room.coin_move_timers[i]);
			float x = lerp(origin.x, place.x, t);
			float y = lerp(origin.y, place.y, 3*t*t - 2*t);

			CAT_draw_queue_animate(coin_world_sprite, 2, x, y, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_BOTTOM);
			CAT_draw_queue_animate(coin_world_sprite, 2, x, y, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_BOTTOM);
		}

		int icon_mode = CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y;
		CAT_draw_queue_add(icon_feed_sprite, 0, 3, 8+16, 280+16, icon_mode); 
		CAT_draw_queue_add(icon_study_sprite, 0, 3, 56+16, 280+16, icon_mode); 
		CAT_draw_queue_add(icon_play_sprite, 0, 3, 104+16, 280+16, icon_mode);
		CAT_draw_queue_add(icon_deco_sprite, 0, 3, 152+16, 280+16, icon_mode);
		CAT_draw_queue_add(icon_menu_sprite, 0, 3, 200+16, 280+16, icon_mode);
		CAT_draw_queue_add(button_hl_sprite, 0, 3, 8+16+48*room.selector, 280+16, icon_mode);
		if(input.touch.pressure)
			CAT_draw_queue_add(touch_hl_sprite, 0, 4, input.touch.x, input.touch.y, icon_mode);
	}
}
