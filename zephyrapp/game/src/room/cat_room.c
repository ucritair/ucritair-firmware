#include "cat_room.h"

#include "cat_input.h"
#include "cat_item.h"
#include <stdio.h>
#include "cat_pet.h"
#include "cat_render.h"
#include "cat_gui.h"
#include <math.h>
#include "cat_menu.h"
#include "cat_actions.h"
#include "cat_deco.h"
#include "cat_arcade.h"
#include "theme_assets.h"
#include "cat_aqi.h"
#include "sprite_assets.h"
#include "item_assets.h"
#include "cat_crisis.h"
#include "cat_monitors.h"
#include "cat_notices.h"
#include "cat_gizmos.h"
#include "cat_curves.h"

//////////////////////////////////////////////////////////////////////////
// SPACE

typedef struct CAT_cell
{
	int idx;
	CAT_ivec2 coords;
	
	bool occupied;
	bool visited;
} CAT_cell;

static CAT_cell cells[CAT_ROOM_GRID_SIZE];
static int free_cell_count = CAT_ROOM_GRID_SIZE;

static int idx_queue[CAT_ROOM_GRID_SIZE];
static int idx_queue_length = 0;

void idx_enqueue(int idx)
{
	if(idx_queue_length >= CAT_ROOM_GRID_SIZE)
		return;

	idx_queue[idx_queue_length] = idx;
	idx_queue_length += 1;
}

int idx_dequeue()
{
	if(idx_queue_length <= 0)
		return -1;

	int idx = idx_queue[0];
	idx_queue_length -= 1;
	for(int i = 0; i < idx_queue_length; i++)
	{
		idx_queue[i] = idx_queue[i+1];
	}
	return idx;
}

CAT_ivec2 CAT_grid2world(CAT_ivec2 grid)
{
	int x = (grid.x + CAT_ROOM_GRID_X) * CAT_TILE_SIZE;
	int y = (grid.y + CAT_ROOM_GRID_Y) * CAT_TILE_SIZE;
	return (CAT_ivec2) {x, y};
}

CAT_ivec2 CAT_world2grid(CAT_ivec2 world)
{
	int x = world.x / CAT_TILE_SIZE - CAT_ROOM_GRID_X;
	int y = world.y / CAT_TILE_SIZE - CAT_ROOM_GRID_Y;
	return (CAT_ivec2) {x, y};
}

CAT_cell* CAT_get_cell(CAT_ivec2 coords)
{
	if(coords.x < 0 || coords.x >= CAT_ROOM_GRID_W)
		return NULL;
	if(coords.y < 0 || coords.y >= CAT_ROOM_GRID_H)
		return NULL;

	int idx = coords.y * CAT_ROOM_GRID_W + coords.x;
	return &cells[idx];
}

bool CAT_is_grid_point_free(CAT_ivec2 point)
{
	if(point.x < 0 || point.x >= CAT_ROOM_GRID_W)
		return false;
	if(point.y < 0 || point.y >= CAT_ROOM_GRID_H)
		return false;
	return !cells[point.y * CAT_ROOM_GRID_W + point.x].occupied;
}

bool CAT_is_world_point_free(CAT_vec2 point)
{
	CAT_ivec2 pixel = {point.x, point.y};
	CAT_ivec2 grid = CAT_world2grid(pixel);
	return CAT_is_grid_point_free(grid);
}

bool CAT_is_block_free(CAT_rect block)
{
	if(block.min.x < 0 || block.max.x > CAT_ROOM_GRID_W)
		return false;
	if(block.min.y < 0 || block.max.y > CAT_ROOM_GRID_H)
		return false;
	
	for(int y = block.min.y; y < block.max.y; y++)
	{
		for(int x = block.min.x; x < block.max.x; x++)
		{
			int idx = y * CAT_ROOM_GRID_W + x;
			if(cells[idx].occupied)
				return false;
		}
	}
	return true;
}

void CAT_toggle_block(CAT_rect block, bool value)
{
	if(block.min.x < 0 || block.max.x > CAT_ROOM_GRID_W)
		return;
	if(block.min.y < 0 || block.max.y > CAT_ROOM_GRID_H)
		return;

	for(int y = block.min.y; y < block.max.y; y++)
	{
		for(int x = block.min.x; x < block.max.x; x++)
		{
			int idx = y * CAT_ROOM_GRID_W + x;
			if(cells[idx].occupied != value)
			{
				cells[idx].occupied = value;
				free_cell_count += value ? -1 : 1;
			}
		}
	}
}

bool CAT_has_free_space()
{
	return free_cell_count > 0;
}

CAT_ivec2 CAT_first_free_space()
{
	if(free_cell_count <= 0)
	{
		CAT_printf("[WARNING] No free space\n");
		return (CAT_ivec2) {-1, -1};
	}

	for(int i = 0; i < CAT_ROOM_GRID_SIZE; i++)
	{
		if(!cells[i].occupied)
			return cells[i].coords;
	}
	return (CAT_ivec2) {-1, -1};
}

CAT_ivec2 CAT_rand_free_space()
{
	if(free_cell_count <= 0)
	{
		CAT_printf("[WARNING] No free space\n");
		return (CAT_ivec2) {-1, -1};
	}

	idx_queue_length = 0;
	for(int i = 0; i < CAT_ROOM_GRID_SIZE; i++)
	{
		if(!cells[i].occupied)
			idx_enqueue(i);
	}

	int idx = idx_queue[CAT_rand_int(0, idx_queue_length-1)];
	return cells[idx].coords;
}

CAT_ivec2 CAT_nearest_free_space(CAT_ivec2 cell)
{
	if(free_cell_count <= 0)
	{
		CAT_printf("[WARNING] No free space\n");
		return (CAT_ivec2) {-1, -1};
	}

	int depth = 0;
	int depth_max = max(CAT_ROOM_GRID_W, CAT_ROOM_GRID_H) / 2 + 1;

	while(depth < depth_max)
	{
		for(int dy = -depth; dy <= depth; dy++)
		{
			int y = cell.y + dy;
			if(y < 0 || y >= CAT_ROOM_GRID_H)
				continue;

			for(int dx = -depth; dx <= depth; dx++)
			{
				int x = cell.x + dx;
				if(x < 0 || x >= CAT_ROOM_GRID_W)
					continue;
				if(abs(dx) + abs(dy) != depth)
					continue;

				int idx = y * CAT_ROOM_GRID_W + x;
				if(!cells[idx].occupied)
					return (CAT_ivec2) {x, y};
			}
		}
		depth++;
	}

	return (CAT_ivec2) {-1, -1};
}

bool is_cell_clean(CAT_ivec2 coords)
{
	if(coords.x < 0 || coords.x >= CAT_ROOM_GRID_W)
		return false;
	if(coords.y < 0 || coords.y >= CAT_ROOM_GRID_H)
		return false;

	int idx = coords.y * CAT_ROOM_GRID_W + coords.x;
	return !(cells[idx].occupied || cells[idx].visited);
}

CAT_ivec2 CAT_largest_free_space()
{
	if(free_cell_count <= 0)
	{
		CAT_printf("[WARNING] No free space\n");
		return (CAT_ivec2) {-1, -1};
	}
	
	for(int i = 0; i < CAT_ROOM_GRID_SIZE; i++)
	{
		cells[i].visited = false;
	}
	
	int max_size = 0;
	CAT_ivec2 max_cent = {0, 0};
	
	for(int y = 0; y < CAT_ROOM_GRID_H; y++)
	{
		for(int x = 0; x < CAT_ROOM_GRID_W; x++)
		{
			CAT_ivec2 coords = {x, y};
			int idx = y * CAT_ROOM_GRID_W + x;

			if(CAT_get_cell(coords)->occupied)
				continue;
			if(CAT_get_cell(coords)->visited)
				continue;
			
			idx_queue_length = 0;
			idx_enqueue(idx);
			int size = 0;
			CAT_ivec2 cent = {0, 0};

			while(idx_queue_length > 0)
			{		
				CAT_cell* c = &cells[idx_dequeue()];
				size += 1;
				cent = CAT_ivec2_add(cent, c->coords);

				CAT_ivec2 n = {c->coords.x, c->coords.y-1};
				int n_idx = n.y * CAT_ROOM_GRID_W + n.x;
				if(is_cell_clean(n))
				{
					cells[n_idx].visited = true;
					idx_enqueue(n_idx);
				}
				CAT_ivec2 e = {c->coords.x+1, c->coords.y};
				int e_idx = e.y * CAT_ROOM_GRID_W + e.x;
				if(is_cell_clean(e))
				{
					cells[e_idx].visited = true;
					idx_enqueue(e_idx);
				}
				CAT_ivec2 s = {c->coords.x, c->coords.y+1};
				int s_idx = s.y * CAT_ROOM_GRID_W + s.x;
				if(is_cell_clean(s))
				{
					cells[s_idx].visited = true;
					idx_enqueue(s_idx);
				}	
				CAT_ivec2 w = {c->coords.x-1, c->coords.y};
				int w_idx = w.y * CAT_ROOM_GRID_W + w.x;
				if(is_cell_clean(w))
				{
					cells[w_idx].visited = true;
					idx_enqueue(w_idx);
				}
			}

			if(size > max_size)
			{
				max_size = size;
				max_cent = CAT_ivec2_div(cent, size);
			}
		}
	}

	return max_cent;
}

bool CAT_is_point_free(CAT_vec2 point)
{
	CAT_ivec2 grid_point = CAT_world2grid((CAT_ivec2) {point.x, point.y});
	CAT_rect point_rect = CAT_rect_place(grid_point, (CAT_ivec2) {1, 1});
	return CAT_is_block_free(point_rect);
}


//////////////////////////////////////////////////////////////////////////
// ROOM

CAT_room room =
{
	.theme = &basic_theme,
	.prop_count = 0,
	.pickup_count = 0
};

static CAT_datetime datetime;

void CAT_room_init()
{
	for(int y = 0; y < CAT_ROOM_GRID_H; y++)
	{
		for(int x = 0; x < CAT_ROOM_GRID_W; x++)
		{
			int idx = y * CAT_ROOM_GRID_W + x;
			cells[idx].idx = idx;
			cells[idx].coords = (CAT_ivec2) {x, y};
			cells[idx].occupied = false;
			cells[idx].visited = false;
		}
	}
	free_cell_count = CAT_ROOM_GRID_SIZE;

	room.theme = &basic_theme;
	room.prop_count = 0;
	room.pickup_count = 0;
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

int CAT_room_find_spatial(CAT_ivec2 place)
{
	for(int i = 0; i < room.prop_count; i++)
	{
		CAT_rect prop_rect = room.prop_rects[i];
		CAT_rect place_rect = CAT_rect_place(place, (CAT_ivec2) {1, 1});
		if(CAT_rect_contains(prop_rect, place_rect))
			return i;
	}
	return -1;
}

bool CAT_prop_fits(int item_id, CAT_ivec2 place)
{
	if(room.prop_count >= CAT_ROOM_GRID_SIZE)
		return false;
	CAT_item* prop = CAT_item_get(item_id);
	if(prop == NULL)
		return false;

	CAT_rect prop_rect = CAT_rect_place(place, prop->prop_shape);
	return CAT_is_block_free(prop_rect);
}

int CAT_room_add_prop(int item_id, CAT_ivec2 place)
{
	if(room.prop_count >= CAT_ROOM_GRID_SIZE)
		return -1;
	CAT_item* prop = CAT_item_get(item_id);
	if(prop == NULL)
		return -1;

	CAT_rect prop_rect = CAT_rect_place(place, prop->prop_shape);
	CAT_toggle_block(prop_rect, true);

	int idx = room.prop_count;
	room.prop_count += 1;
	room.prop_ids[idx] = item_id;
	room.prop_rects[idx] = prop_rect;
	room.prop_overrides[idx] = 0;
	room.prop_children[idx] = -1;
	return idx;
}

void CAT_room_stack_prop(int idx, int item_id)
{
	room.prop_children[idx] = item_id;
}

void CAT_room_unstack_prop(int idx)
{
	room.prop_children[idx] = -1;
}

void CAT_room_remove_prop(int idx)
{
	if(idx < 0 || idx >= room.prop_count)
		return;

	CAT_rect prop_rect = room.prop_rects[idx];
	CAT_toggle_block(prop_rect, false);

	if(room.prop_children[idx] != -1)
		CAT_inventory_add(room.prop_children[idx], 1);

	room.prop_count -= 1;
	for(int i = idx; i < room.prop_count; i++)
	{
		room.prop_ids[i] = room.prop_ids[i+1];
		room.prop_rects[i] = room.prop_rects[i+1];
		room.prop_overrides[i] = room.prop_overrides[i+1];
		room.prop_children[i] = room.prop_children[i+1];
	}
}

void CAT_room_flip_prop(int idx)
{
	if(idx < 0 || idx >= room.prop_count)
		return;

	int prop_id = room.prop_ids[idx];
	CAT_item* prop = CAT_item_get(prop_id);
	int* override = &room.prop_overrides[idx];

	if(prop->prop_animated || prop->sprite->frame_count == 1)
	{
		*override = !(*override);
	}
	else
	{
		*override += 1;
		if(*override >= prop->sprite->frame_count)
			*override = 0;
	}
}

#define PICKUP_AIR_TIME 0.75f

int CAT_spawn_pickup(CAT_ivec2 origin, CAT_ivec2 place, const CAT_sprite* sprite, void (*proc)())
{
	if(room.pickup_count >= CAT_MAX_PICKUP_COUNT)
		return -1;
	
	int idx = room.pickup_count;
	room.pickup_count += 1;
	room.pickups[idx].origin = origin;
	room.pickups[idx].place = place;
	room.pickups[idx].sprite = sprite;
	room.pickups[idx].proc = proc;
	room.pickups[idx].timer = 0;

	return idx;
}

void CAT_despawn_pickup(int idx)
{
	if(idx < 0 || idx >= room.pickup_count)
		return;
	
	for(int i = idx; i < room.pickup_count-1; i++)
		room.pickups[i] = room.pickups[i+1];
	room.pickup_count -=1;
}

static CAT_machine_state button_modes[5] =
{
	CAT_MS_feed,
	CAT_MS_study,
	CAT_MS_play,
	CAT_MS_deco,
	CAT_MS_menu
};
static int mode_selector = 0;

void CAT_room_tick()
{
	for(int i = 0; i < room.pickup_count; i++)
	{
		if(room.pickups[i].timer < PICKUP_AIR_TIME)
			room.pickups[i].timer += CAT_get_delta_time_s();
	}

	for(int i = 0; i < room.prop_count; i++)
	{
		if
		(
			room.prop_ids[i] == prop_vig_flower_item ||
			room.prop_ids[i] == prop_foc_flower_item ||
			room.prop_ids[i] == prop_spi_flower_item
		)
		{
			float aqi_score = CAT_AQ_aggregate_score();
			if(aqi_score < 15)
			{
				room.prop_overrides[i] = 5;
			}
			else
			{
				int aqi_idx = round(aqi_score / 100.0f * 4.0f);
				aqi_idx = clamp(aqi_idx, 0, 4);
				room.prop_overrides[i] = aqi_idx;
			}
		}
	}
}

#define MODE_BUTTON_COUNT 5
#define MODE_BUTTONS_X 16
#define MODE_BUTTONS_Y 280
#define MODE_BUTTON_SIZE 32
#define MODE_BUTTON_PAD 12

#define ARCADE_X 128
#define ARCADE_Y 48
#define ARCADE_W 32
#define ARCADE_H 64

#define VENDING_MACHINE_X 176
#define VENDING_MACHINE_Y 16
#define VENDING_MACHINE_W 56
#define VENDING_MACHINE_H 96

void mode_button_input()
{
	if(CAT_input_pressed(CAT_BUTTON_RIGHT))
		mode_selector += 1;
	if(CAT_input_pressed(CAT_BUTTON_LEFT))
		mode_selector -= 1;
	mode_selector = (mode_selector + MODE_BUTTON_COUNT) % MODE_BUTTON_COUNT;
	if(CAT_input_pressed(CAT_BUTTON_A))
		CAT_machine_transition(button_modes[mode_selector]);
	
	for(int i = 0; i < MODE_BUTTON_COUNT; i++)
	{
		if(CAT_input_touch_rect
		(
			MODE_BUTTONS_X + (MODE_BUTTON_SIZE + MODE_BUTTON_PAD) * i,
			MODE_BUTTONS_Y,
			MODE_BUTTON_SIZE, MODE_BUTTON_SIZE
		))
		{
			if(i == mode_selector)
				CAT_machine_transition(button_modes[mode_selector]);
			mode_selector = i;
		}
	}
}

void world_button_input()
{
	if(CAT_input_touch_rect(VENDING_MACHINE_X, VENDING_MACHINE_Y, VENDING_MACHINE_W, VENDING_MACHINE_H))
				CAT_machine_transition(CAT_MS_shop);
	else if(CAT_input_touch_rect(ARCADE_X, ARCADE_Y, ARCADE_W, ARCADE_H))
		CAT_machine_transition(CAT_MS_arcade);
	else if(CAT_input_touch_rect
	(
		room.theme->window_rect.min.x+4,
		room.theme->window_rect.min.y+4,
		room.theme->window_rect.max.x-room.theme->window_rect.min.x-8,
		room.theme->window_rect.max.y-room.theme->window_rect.min.y-8
	))
	{
		CAT_machine_transition(CAT_MS_monitor);
	}
}

void pickup_input()
{
	for(int i = 0; i < room.pickup_count; i++)
	{
		if(room.pickups[i].timer >= PICKUP_AIR_TIME)
		{
			CAT_ivec2 place = room.pickups[i].place;
			if(CAT_input_drag(place.x + 8, place.y - 8, 16))
			{
				room.pickups[i].proc();
				CAT_despawn_pickup(i);
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
			CAT_set_render_callback(CAT_render_room);
			CAT_pet_settle();
			CAT_animator_reset(&prop_hoopy_sprite);
			CAT_gui_dismiss_dialogue();
			CAT_gui_reset_menu_context();
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_get_datetime(&datetime);

			if(CAT_AQ_is_crisis_report_posted())
			{
				CAT_machine_transition(CAT_MS_crisis_report);
				return;
			}
			if(CAT_pet_is_death_report_posted())
			{
				CAT_machine_transition(CAT_MS_death_report);
				return;
			}
				
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_menu);

			if(CAT_pet_is_dead())
			{
				if(CAT_input_touch_rect(120-32, 228-80, 64, 80))
				{
					CAT_pet_reincarnate();
				}

				world_button_input();
				pickup_input();
			}
			else
			{
				mode_button_input();
				world_button_input();
				pickup_input();

				CAT_pet_update_animations();
				CAT_pet_walk();
				CAT_pet_react();
			}

			if(CAT_should_post_notice())
			{
				CAT_clear_notice_types();
				CAT_enable_notice_type(CAT_NOTICE_TYPE_MISCELLANY);

				if(CAT_pet_is_dead())
					CAT_enable_notice_type(CAT_NOTICE_TYPE_DEAD);
				else
				{
					if(pet.vigour < 6 && pet.focus < 6 && pet.spirit < 6)
						CAT_enable_notice_type(CAT_NOTICE_TYPE_STATS_BAD);
					else if(pet.vigour > 6 && pet.focus > 6 && pet.spirit > 6)
						CAT_enable_notice_type(CAT_NOTICE_TYPE_STATS_GOOD);

					if(CAT_AQ_aggregate_score() <= 60)
						CAT_enable_notice_type(CAT_NOTICE_TYPE_AQ_BAD);
					else if(CAT_AQ_aggregate_score() > 80)
						CAT_enable_notice_type(CAT_NOTICE_TYPE_AQ_GOOD);

					if(datetime.hour >= 4 && datetime.hour < 11)
						CAT_enable_notice_type(CAT_NOTICE_TYPE_MORNING);
					else if(datetime.hour >= 11 && datetime.hour < 20)
						CAT_enable_notice_type(CAT_NOTICE_TYPE_DAY);
					else if(datetime.hour >= 20)
						CAT_enable_notice_type(CAT_NOTICE_TYPE_NIGHT);
					
					if(datetime.month <= 3)
						CAT_enable_notice_type(CAT_NOTICE_TYPE_SPRING);
					else if(datetime.month <= 6)
						CAT_enable_notice_type(CAT_NOTICE_TYPE_SUMMER);
					else if(datetime.month <= 9)
						CAT_enable_notice_type(CAT_NOTICE_TYPE_AUTUMN);
					else
						CAT_enable_notice_type(CAT_NOTICE_TYPE_WINTER);
				}

				const char* notice = CAT_pick_notice(CAT_pick_notice_type());
				CAT_post_notice(notice);
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			CAT_gui_dismiss_dialogue();
			break;
		}
	}
}

void CAT_room_draw_statics()
{
	int window_y = room.theme->window_rect.min.y;
	int window_x = room.theme->window_rect.min.x;
	int window_width = room.theme->window_rect.max.x-room.theme->window_rect.min.x;
	int window_height = room.theme->window_rect.max.y-room.theme->window_rect.min.y;
	int window_columns = window_width/8;
	int sky_row_off = datetime.hour * (480 - window_height) / 23;

	if(room.theme->tile_wall)
	{
		for(int y = 0; y < 6; y++)
		{
			for(int x = 0; x < 15; x++)
			{
				int tile_idx = room.theme->wall_map[y * 15 + x];
				CAT_draw_tile(room.theme->wall_tiles, tile_idx, x * 16, y * 16);
			}
		}

		for(int i = 0; i < window_columns; i++)
		{
			CAT_set_sprite_mask(window_x+4, window_y+4, window_x+window_width-4, window_y+window_height-4);
			CAT_draw_sprite_raw(&sky_gradient_sprite, 0, window_x + i * 8, window_y - sky_row_off);
		}
		CAT_draw_sprite_raw(&window_sprite, 0, window_x, window_y);
	}
	else
	{
		for(int i = 0; i < window_columns; i++)
		{
			CAT_set_sprite_mask(window_x, window_y, window_x+window_width, window_y+window_height);
			CAT_draw_sprite_raw(&sky_gradient_sprite, 0, window_x + i * 8, window_y - sky_row_off);
		}
		CAT_draw_background(room.theme->wall_tiles, 0, 0);
	}

	if(room.theme->tile_floor)
	{
		for(int y = 0; y < 14; y++)
		{
			for(int x = 0; x < 15; x++)
			{
				int tile_idx = room.theme->floor_map[y * 15 + x];
				CAT_draw_tile(room.theme->floor_tiles, tile_idx, x * 16, (6 + y) * 16);
			}
		}
	}
	else
	{
		int row_offset = room.theme->tile_wall ?
		16*16 : room.theme->wall_tiles->height;
		CAT_draw_background(room.theme->floor_tiles, 0, row_offset);
	}

	CAT_draw_sprite_raw(&vending_sprite, -1, 172, 16);
	CAT_draw_sprite_raw(&arcade_sprite, -1, 124, 48);

	int battery_x = window_width == 240 ?
	196 : window_x+window_width/2;
	int battery_y = window_width == 240 ?
	62 : window_y+window_height/2 - 2;
	if(CAT_is_charging())
	{
		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		CAT_draw_sprite_raw(&icon_charging_sprite, 0, battery_x, battery_y);
	}
	else if(CAT_get_persist_flag(CAT_PERSIST_FLAG_BATTERY_ALERT))
	{
		if(CAT_pulse(0.25f))
		{
			CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
			CAT_draw_sprite_raw(&icon_low_battery_alt_sprite, 0, battery_x, battery_y);
		}
	}

	int alarm_x = window_width == 240 ?
	80 : window_x+window_width/2;
	int alarm_y = window_width == 240 ?
	38 : window_y+window_height/2 - 2;
	if(CAT_AQ_is_crisis_ongoing())
	{
		for(int i = 0; i < 3; i++)
		{
			float base_t = CAT_get_uptime_ms() / 1000.0f / 6.28;
			CAT_draw_gizmo_primitive(CAT_GIZMO_PRIMITIVE_HEX, alarm_x, alarm_y, 24 + i * 4 + 4 * sin(base_t * 6.28 * 4), base_t - i * 0.15f, CAT_RED);
		}
	}
	else
	{
		float base_t = CAT_get_uptime_ms() / 1000.0f / 6.28;
		CAT_draw_gizmo_primitive(CAT_GIZMO_PRIMITIVE_HEX, alarm_x, alarm_y, 24, base_t * 0.15f, CAT_MONITOR_BLUE);
	}
}

void CAT_room_draw_props()
{
	for(int i = 0; i < room.prop_count; i++)
	{
		int prop_id = room.prop_ids[i];
		CAT_item* prop = CAT_item_get(prop_id);
		CAT_ivec2 shape = prop->prop_shape;
		CAT_ivec2 place = room.prop_rects[i].min;

		CAT_ivec2 draw_place = CAT_grid2world(place);
		draw_place.y += shape.y * CAT_TILE_SIZE;

		int flags = CAT_DRAW_FLAG_BOTTOM;
		int frame_idx = 0;
		if(room.prop_overrides[i])
		{
			if(prop->prop_animated || prop->sprite->frame_count == 1)
				flags |= CAT_DRAW_FLAG_REFLECT_X;
			else
				frame_idx = room.prop_overrides[i];
		}
		int job = prop->prop_animated ?
		CAT_draw_queue_add(prop->sprite, -1, PROPS_LAYER, draw_place.x, draw_place.y, flags) :
		CAT_draw_queue_add(prop->sprite, frame_idx, PROPS_LAYER, draw_place.x, draw_place.y, flags);
		
		CAT_item* child = CAT_item_get(room.prop_children[i]);
		if(child != NULL)
		{
			flags = CAT_DRAW_FLAG_BOTTOM | CAT_DRAW_FLAG_CENTER_X;
			frame_idx = child->prop_animated ? -1 : 0;
			int cx = (shape.x / 2) * CAT_TILE_SIZE;
			int cy = shape.y * CAT_TILE_SIZE;
			int dy = child->prop_child_dy;
			CAT_draw_queue_insert(job+1, child->sprite, frame_idx, PROPS_LAYER, draw_place.x + cx, draw_place.y - cy - dy, flags);
		}
	}
}

void CAT_room_draw_pickups()
{
	for(int i = 0; i < room.pickup_count; i++)
	{
		CAT_ivec2 origin = room.pickups[i].origin;
		CAT_ivec2 place = room.pickups[i].place;

		float t = min(room.pickups[i].timer / PICKUP_AIR_TIME, 1.0f);
		float x = lerp(origin.x, place.x, t);
		if(origin.y > place.y)
			t = 1.0f - t;
		float y = lerp(origin.y, place.y, 3*t*t - 2*t);
		if(origin.y > place.y)
			y = -y + origin.y + place.y;

		CAT_draw_queue_add(room.pickups[i].sprite, -1, PROPS_LAYER, x, y, CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_BOTTOM);
	}
}

void CAT_room_draw_pet()
{
	CAT_anim_tick(&AM_pet);
	CAT_anim_tick(&AM_mood);

	if(CAT_pet_is_dead())
	{
		CAT_draw_queue_add(&grave_egg_sprite, 0, PROPS_LAYER, 120, 228, CAT_DRAW_FLAG_BOTTOM | CAT_DRAW_FLAG_CENTER_X);
	}
	else
	{
		int flags = CAT_DRAW_FLAG_BOTTOM | CAT_DRAW_FLAG_CENTER_X;
		if(pet.rot != 1)
			flags |= CAT_DRAW_FLAG_REFLECT_X;
		int layer = CAT_get_battery_pct() <= CAT_CRITICAL_BATTERY_PCT ? PROPS_LAYER-1 : PROPS_LAYER;

		CAT_draw_queue_add(CAT_anim_read(&AM_pet), -1, layer, pet.pos.x, pet.pos.y, flags);

		if(CAT_anim_is_in(&AM_mood, &AS_react))
		{
			int x_off = pet.rot != 1 ? 16 : -16;
			CAT_draw_queue_add(CAT_anim_read(&AM_mood), -1, layer+1, pet.pos.x + x_off, pet.pos.y - 48, flags);	
		}
	}
}

static const CAT_sprite* button_sprites[] =
{
	&icon_feed_sprite,
	&icon_study_sprite,
	&icon_play_sprite,
	&icon_deco_sprite,
	&icon_menu_sprite
};

void CAT_room_draw_gui()
{
	if(CAT_pet_is_dead())
		return;

	for(int i = 0; i < sizeof(button_sprites)/sizeof(button_sprites[0]); i++)
	{
		CAT_draw_queue_add
		(
			button_sprites[i], 0, GUI_LAYER,
			MODE_BUTTONS_X + (MODE_BUTTON_SIZE + MODE_BUTTON_PAD) * i,
			MODE_BUTTONS_Y,
			CAT_DRAW_FLAG_NONE
		);
	}
	CAT_strokeberry
	(
		MODE_BUTTONS_X + (MODE_BUTTON_SIZE + MODE_BUTTON_PAD) * mode_selector,
		MODE_BUTTONS_Y,
		MODE_BUTTON_SIZE+1, MODE_BUTTON_SIZE+1,
		CAT_WHITE
	);

	if(input.touch.pressure)
		CAT_circberry(input.touch.x, input.touch.y, 16, CAT_WHITE);
}

void CAT_render_room()
{
	CAT_room_draw_statics();

	if (CAT_get_render_cycle() == 0)
		CAT_draw_queue_clear();
	CAT_room_draw_props();
	CAT_room_draw_pickups();
	CAT_room_draw_pet();
	CAT_draw_queue_submit();

	CAT_room_draw_gui();
}
