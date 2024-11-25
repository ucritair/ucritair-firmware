#include "cat_room.h"

#include "cat_input.h"
#include "cat_item.h"
#include <stdio.h>
#include "cat_pet.h"
#include "cat_sprite.h"
#include "cat_gui.h"
#include "cat_bag.h"
#include <math.h>
#include "cat_menu.h"
#include "cat_actions.h"
#include "cat_deco.h"
#include "cat_vending.h"
#include "cat_arcade.h"

CAT_space space;

void CAT_space_init()
{
	space.grid_place = (CAT_ivec2) {0, 7};
	space.grid_shape = (CAT_ivec2) {15, 10};

	space.world_shape = CAT_ivec2_mul(space.grid_shape, 16);
	space.world_rect.min = CAT_ivec2_mul(space.grid_place, 16);
	space.world_rect.max = CAT_ivec2_add(space.world_rect.min, space.world_shape);

	for(int y = 0; y < space.grid_shape.y; y++)
	{
		for(int x = 0; x < space.grid_shape.x; x++)
		{
			int idx = y * space.grid_shape.x + x;
			space.cells[idx] = 0;
			space.free_list[idx] = (CAT_ivec2) {x, y};
		}
	}
	space.free_list_length = CAT_GRID_SIZE;
}

CAT_ivec2 CAT_grid2world(CAT_ivec2 grid)
{
	int x = (space.grid_place.x + grid.x) * 16;
	int y = (space.grid_place.y + grid.y) * 16;
	return (CAT_ivec2) {x, y};
}

CAT_ivec2 CAT_world2grid(CAT_ivec2 world)
{
	int x = world.x / 16 - space.grid_place.x;
	int y = world.x / 16 - space.grid_place.y;
	return (CAT_ivec2) {x, y};
}

int CAT_get_cell(CAT_ivec2 cell)
{
	int idx = cell.y * space.grid_shape.x + cell.x;
	return space.cells[idx];
}
void CAT_set_cell(CAT_ivec2 cell, int colour)
{
	int idx = cell.y * space.grid_shape.x + cell.x;
	space.cells[idx] = colour;
}

bool CAT_block_free(CAT_rect block)
{
	if(block.min.x < 0 || block.max.x > space.grid_shape.x)
		return false;
	if(block.min.y < 0 || block.max.y > space.grid_shape.y)
		return false;
	
	for(int y = block.min.y; y < block.max.y; y++)
	{
		for(int x = block.min.x; x < block.max.x; x++)
		{
			int idx = y * space.grid_shape.x + x;
			if(space.cells[idx] != 0)
				return false;
		}
	}
	return true;
}

void CAT_set_block(CAT_rect block, int colour)
{
	if(block.min.x < 0 || block.max.x >= space.grid_shape.x)
		return;
	if(block.min.y < 0 || block.max.y >= space.grid_shape.y)
		return;

	for(int y = block.min.y; y < block.max.y; y++)
	{
		for(int x = block.min.x; x < block.max.x; x++)
		{
			int idx = y * space.grid_shape.x + x;
			space.cells[idx] = colour;
		}
	}
}

void CAT_build_free_list()
{
	for(int y = 0; y < space.grid_shape.y; y++)
	{
		for(int x = 0; x < space.grid_shape.x; x++)
		{
			int idx = y * space.grid_shape.x + x;
			if(space.cells[idx] == 0)
			{
				space.free_list[space.free_list_length] = (CAT_ivec2) {x, y};
				space.free_list_length += 1;
			}
		}
	}
}

bool CAT_has_free_space()
{
	return space.free_list_length > 0;
}

CAT_ivec2 CAT_first_free_space()
{
	if(space.free_list_length <= 0)
		return (CAT_ivec2) {-1, -1};
	return space.free_list[0];
}

CAT_ivec2 CAT_rand_free_space()
{
	if(space.free_list_length <= 0)
		return (CAT_ivec2) {-1, -1};
	int idx = CAT_rand_int(0, space.free_list_length - 1);
	return space.free_list[idx];
}

CAT_room room =
{
	.grid_cursor = {7, 5},

	.prop_count = 0,

	.coin_count = 0,

	.button_modes =
	{
		CAT_MS_feed,
		CAT_MS_study,
		CAT_MS_play,
		CAT_MS_deco,
		CAT_MS_menu
	},
	.mode_selector = 0
};

void CAT_room_init()
{
	for(int i = 0; i < CAT_MAX_COIN_COUNT; i++)
	{
		room.coin_move_timers[i] = CAT_timer_init(0.75f);
	}
	room.earn_timer_id = CAT_timer_init(CAT_EARN_TICK_SECS);
}

int CAT_room_find(int item_id)
{
	for(int i = 0; i < room.prop_count; i++)
	{
		if(room.prop_ids[i] == item_id)
			return i;
	}
	return -1;
}

bool CAT_prop_fits(int item_id, CAT_ivec2 place)
{
	if(room.prop_count >= CAT_GRID_SIZE)
		return false;
	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return false;

	CAT_ivec2 shape = item->data.prop_data.shape;
	CAT_rect block = CAT_rect_place(place, shape);
	return CAT_block_free(block);
}

int CAT_room_add_prop(int item_id, CAT_ivec2 place)
{
	if(room.prop_count >= CAT_GRID_SIZE)
		return -1;
	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return -1;

	CAT_ivec2 shape = item->data.prop_data.shape;
	CAT_rect block = CAT_rect_place(place, shape);
	CAT_set_block(block, 1);
	CAT_build_free_list();

	int idx = room.prop_count;
	room.prop_count += 1;
	room.prop_ids[idx] = item_id;
	room.prop_places[idx] = place;
	room.prop_overrides[idx] = 0;
	return idx;
}

void CAT_room_remove_prop(int idx)
{
	if(idx < 0 || idx >= room.prop_count)
		return;

	CAT_item* item = CAT_item_get(room.prop_ids[idx]);
	CAT_ivec2 shape = item->data.prop_data.shape;
	CAT_rect block = CAT_rect_place(room.prop_places[idx], shape);
	CAT_set_block(block, 0);
	CAT_build_free_list();

	room.prop_count -= 1;
	for(int i = idx; i < room.prop_count; i++)
	{
		room.prop_ids[i] = room.prop_ids[i+1];
		room.prop_places[i] = room.prop_places[i+1];
		room.prop_overrides[i] = room.prop_overrides[i+1];
	}
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
				CAT_ivec2 start = CAT_grid2world(room.prop_places[i]);
				start.x += 24;
				start.y -= 24;

				CAT_ivec2 end_grid = CAT_rand_free_space();
				CAT_ivec2 end_world = CAT_grid2world(end_grid);

				float xi = start.x;
				float yi = start.y;
				float xf = end_world.x;
				float yf = end_world.y;
				CAT_room_add_coin((CAT_vec2) {xi, yi}, (CAT_vec2) {xf, yf});
			}
		}
	}
}

void CAT_room_cursor()
{
	if(CAT_input_pulse(CAT_BUTTON_UP))
		room.grid_cursor.y -= 1;
	if(CAT_input_pulse(CAT_BUTTON_RIGHT))
		room.grid_cursor.x += 1;
	if(CAT_input_pulse(CAT_BUTTON_DOWN))
		room.grid_cursor.y += 1;
	if(CAT_input_pulse(CAT_BUTTON_LEFT))
		room.grid_cursor.x -= 1;
	room.grid_cursor.x = clamp(room.grid_cursor.x, 0, space.grid_shape.x-1);
	room.grid_cursor.y = clamp(room.grid_cursor.y, 0, space.grid_shape.y-1);
}

void CAT_room_tick(bool capture_input)
{
	if(CAT_timer_tick(room.earn_timer_id))
	{
		CAT_room_earn(1);
		CAT_timer_reset(room.earn_timer_id);
	}

	for(int i = 0; i < room.prop_count; i++)
	{
		if
		(
			room.prop_ids[i] == flower_vig_item ||
			room.prop_ids[i] == flower_foc_item ||
			room.prop_ids[i] == flower_spi_item
		)
		{
			float aqi_score = CAT_AQI_aggregate() / 100.0f;
			if(aqi_score < 0.15f)
			{
				room.prop_overrides[i] = 5;
			}
			else
			{
				int aqi_idx = round(aqi_score * 4.0f);
				aqi_idx = clamp(aqi_idx, 0, 4);
				room.prop_overrides[i] = aqi_idx;
			}
		}
	}

	if(!capture_input)
		return;

	for(int i = 0; i < 5; i++)
	{
		if(CAT_input_touch(8+16+48*i, 280+16, 16))
		{
			CAT_machine_transition(room.button_modes[i]);
			room.mode_selector = i;
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
				coins += 1;
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
				room.mode_selector += 1;
				if(room.mode_selector > 4)
					room.mode_selector = 0;
			}
			if(CAT_input_pulse(CAT_BUTTON_LEFT))
			{
				room.mode_selector -= 1;
				if(room.mode_selector < 0)
					room.mode_selector = 4;
			}
			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_machine_transition(room.button_modes[room.mode_selector]);
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
		CAT_datetime time;
		CAT_get_datetime(&time);
		if(time.hour >= 4 && time.hour < 9)
			CAT_draw_queue_add(window_dawn_sprite, 0, 2, 8, 8, CAT_DRAW_MODE_DEFAULT);
		else if(time.hour >= 9 && time.hour <= 18)
			CAT_draw_queue_add(window_day_sprite, 0, 2, 8, 8, CAT_DRAW_MODE_DEFAULT);
		else
			CAT_draw_queue_add(window_day_sprite, 0, 2, 8, 8, CAT_DRAW_MODE_DEFAULT);
		
		CAT_draw_queue_add(vending_sprite, -1, 2, 172, 16, CAT_DRAW_MODE_DEFAULT);
		CAT_draw_queue_add(arcade_sprite, -1, 2, 124, 48, CAT_DRAW_MODE_DEFAULT);

		for(int i = 0; i < room.prop_count; i++)
		{
			int prop_id = room.prop_ids[i];
			CAT_item* prop = CAT_item_get(prop_id);
			CAT_ivec2 shape = prop->data.prop_data.shape;
			CAT_ivec2 place = room.prop_places[i];

			CAT_ivec2 draw_place = CAT_grid2world(place);
			draw_place.y += shape.y * 16;

			int prop_mode = CAT_DRAW_MODE_BOTTOM;
			if(prop->data.prop_data.animate)
			{	
				if(room.prop_overrides[i])
					prop_mode |= CAT_DRAW_MODE_REFLECT_X;
				CAT_draw_queue_animate(prop->sprite_id, 2, draw_place.x, draw_place.y, prop_mode);
			}
			else if(atlas.table[prop->sprite_id].frame_count == 1)
			{
				if(room.prop_overrides[i])
					prop_mode |= CAT_DRAW_MODE_REFLECT_X;
				CAT_draw_queue_add(prop->sprite_id, 0, 2, draw_place.x, draw_place.y, prop_mode);
			}
			else
			{
				int frame_idx = room.prop_overrides[i];
				CAT_draw_queue_add(prop->sprite_id, frame_idx, 2, draw_place.x, draw_place.y, prop_mode);
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
		CAT_draw_queue_add(button_hl_sprite, 0, 3, 8+16+48*room.mode_selector, 280+16, icon_mode);
		if(input.touch.pressure)
			CAT_draw_queue_add(touch_hl_sprite, 0, 4, input.touch.x, input.touch.y, icon_mode);
	}
}
