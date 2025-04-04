#include "cat_room.h"

#include "cat_input.h"
#include "cat_item.h"
#include <stdio.h>
#include "cat_pet.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_bag.h"
#include <math.h>
#include "cat_menu.h"
#include "cat_actions.h"
#include "cat_deco.h"
#include "cat_vending.h"
#include "cat_arcade.h"
#include "theme_assets.h"
#include "cat_aqi.h"
#include "sprite_assets.h"

//////////////////////////////////////////////////////////////////////////
// SPACE

int idx_queue[CAT_GRID_SIZE];
int idx_queue_length = 0;

void idx_enqueue(int idx)
{
	if(idx_queue_length >= CAT_GRID_SIZE)
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

CAT_space space;

void CAT_space_init()
{
	space.grid_place = (CAT_ivec2) {0, 7};

	space.world_rect.min = CAT_ivec2_mul(space.grid_place, CAT_TILE_SIZE);
	space.world_rect.max = CAT_ivec2_add(space.world_rect.min, (CAT_ivec2) {CAT_WORLD_WIDTH, CAT_WORLD_HEIGHT});

	for(int y = 0; y < CAT_GRID_HEIGHT; y++)
	{
		for(int x = 0; x < CAT_GRID_WIDTH; x++)
		{
			int idx = y * CAT_GRID_WIDTH + x;
			space.cells[idx].idx = idx;
			space.cells[idx].coords = (CAT_ivec2) {x, y};
			space.cells[idx].occupied = false;
			space.cells[idx].visited = false;
		}
	}
	space.free_cell_count = CAT_GRID_SIZE;
}

CAT_ivec2 CAT_grid2world(CAT_ivec2 grid)
{
	int x = (space.grid_place.x + grid.x) * CAT_TILE_SIZE;
	int y = (space.grid_place.y + grid.y) * CAT_TILE_SIZE;
	return (CAT_ivec2) {x, y};
}

CAT_ivec2 CAT_world2grid(CAT_ivec2 world)
{
	int x = world.x / 16 - space.grid_place.x;
	int y = world.x / 16 - space.grid_place.y;
	return (CAT_ivec2) {x, y};
}

CAT_cell* CAT_get_cell(CAT_ivec2 coords)
{
	if(coords.x < 0 || coords.x >= CAT_GRID_WIDTH)
		return NULL;
	if(coords.y < 0 || coords.y >= CAT_GRID_HEIGHT)
		return NULL;

	int idx = coords.y * CAT_GRID_WIDTH + coords.x;
	return &space.cells[idx];
}

bool CAT_is_block_free(CAT_rect block)
{
	if(block.min.x < 0 || block.max.x > CAT_GRID_WIDTH)
		return false;
	if(block.min.y < 0 || block.max.y > CAT_GRID_HEIGHT)
		return false;
	
	for(int y = block.min.y; y < block.max.y; y++)
	{
		for(int x = block.min.x; x < block.max.x; x++)
		{
			int idx = y * CAT_GRID_WIDTH + x;
			if(space.cells[idx].occupied)
				return false;
		}
	}
	return true;
}

void CAT_toggle_block(CAT_rect block, bool value)
{
	if(block.min.x < 0 || block.max.x > CAT_GRID_WIDTH)
		return;
	if(block.min.y < 0 || block.max.y > CAT_GRID_HEIGHT)
		return;

	for(int y = block.min.y; y < block.max.y; y++)
	{
		for(int x = block.min.x; x < block.max.x; x++)
		{
			int idx = y * CAT_GRID_WIDTH + x;
			if(space.cells[idx].occupied != value)
			{
				space.cells[idx].occupied = value;
				space.free_cell_count += value ? -1 : 1;
			}
		}
	}
}

bool CAT_has_free_space()
{
	return space.free_cell_count > 0;
}

CAT_ivec2 CAT_first_free_space()
{
	if(space.free_cell_count <= 0)
	{
		CAT_printf("[WARNING] No free space\n");
		return (CAT_ivec2) {-1, -1};
	}

	for(int i = 0; i < CAT_GRID_SIZE; i++)
	{
		if(!space.cells[i].occupied)
			return space.cells[i].coords;
	}
	return (CAT_ivec2) {-1, -1};
}

CAT_ivec2 CAT_rand_free_space()
{
	if(space.free_cell_count <= 0)
	{
		CAT_printf("[WARNING] No free space\n");
		return (CAT_ivec2) {-1, -1};
	}

	idx_queue_length = 0;
	for(int i = 0; i < CAT_GRID_SIZE; i++)
	{
		if(!space.cells[i].occupied)
			idx_enqueue(i);
	}

	int idx = idx_queue[CAT_rand_int(0, idx_queue_length-1)];
	return space.cells[idx].coords;
}

CAT_ivec2 CAT_nearest_free_space(CAT_ivec2 cell)
{
	if(space.free_cell_count <= 0)
	{
		CAT_printf("[WARNING] No free space\n");
		return (CAT_ivec2) {-1, -1};
	}

	int depth = 0;
	int depth_max = max(CAT_GRID_WIDTH, CAT_GRID_HEIGHT) / 2 + 1;

	while(depth < depth_max)
	{
		for(int dy = -depth; dy <= depth; dy++)
		{
			int y = cell.y + dy;
			if(y < 0 || y >= CAT_GRID_HEIGHT)
				continue;

			for(int dx = -depth; dx <= depth; dx++)
			{
				int x = cell.x + dx;
				if(x < 0 || x >= CAT_GRID_WIDTH)
					continue;
				if(abs(dx) + abs(dy) != depth)
					continue;

				int idx = y * CAT_GRID_WIDTH + x;
				if(!space.cells[idx].occupied)
					return (CAT_ivec2) {x, y};
			}
		}
		depth++;
	}

	return (CAT_ivec2) {-1, -1};
}

bool is_cell_clean(CAT_ivec2 coords)
{
	if(coords.x < 0 || coords.x >= CAT_GRID_WIDTH)
		return false;
	if(coords.y < 0 || coords.y >= CAT_GRID_HEIGHT)
		return false;

	int idx = coords.y * CAT_GRID_WIDTH + coords.x;
	return !(space.cells[idx].occupied || space.cells[idx].visited);
}

CAT_ivec2 CAT_largest_free_space()
{
	if(space.free_cell_count <= 0)
	{
		CAT_printf("[WARNING] No free space\n");
		return (CAT_ivec2) {-1, -1};
	}
	
	for(int i = 0; i < CAT_GRID_SIZE; i++)
	{
		space.cells[i].visited = false;
	}
	
	int max_size = 0;
	CAT_ivec2 max_cent = {0, 0};
	
	for(int y = 0; y < CAT_GRID_HEIGHT; y++)
	{
		for(int x = 0; x < CAT_GRID_WIDTH; x++)
		{
			CAT_ivec2 coords = {x, y};
			int idx = y * CAT_GRID_WIDTH + x;

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
				CAT_cell* c = &space.cells[idx_dequeue()];
				size += 1;
				cent = CAT_ivec2_add(cent, c->coords);

				CAT_ivec2 n = {c->coords.x, c->coords.y-1};
				int n_idx = n.y * CAT_GRID_WIDTH + n.x;
				if(is_cell_clean(n))
				{
					space.cells[n_idx].visited = true;
					idx_enqueue(n_idx);
				}
				CAT_ivec2 e = {c->coords.x+1, c->coords.y};
				int e_idx = e.y * CAT_GRID_WIDTH + e.x;
				if(is_cell_clean(e))
				{
					space.cells[e_idx].visited = true;
					idx_enqueue(e_idx);
				}
				CAT_ivec2 s = {c->coords.x, c->coords.y+1};
				int s_idx = s.y * CAT_GRID_WIDTH + s.x;
				if(is_cell_clean(s))
				{
					space.cells[s_idx].visited = true;
					idx_enqueue(s_idx);
				}	
				CAT_ivec2 w = {c->coords.x-1, c->coords.y};
				int w_idx = w.y * CAT_GRID_WIDTH + w.x;
				if(is_cell_clean(w))
				{
					space.cells[w_idx].visited = true;
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


//////////////////////////////////////////////////////////////////////////
// ROOM

CAT_room room;

void CAT_room_init()
{
	room.theme = &base_theme;

	room.prop_count = 0;

	room.pickup_count = 0;
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
	if(room.prop_count >= CAT_GRID_SIZE)
		return false;
	CAT_item* prop = CAT_item_get(item_id);
	if(prop == NULL)
		return false;

	CAT_rect prop_rect = CAT_rect_place(place, prop->data.prop_data.shape);
	return CAT_is_block_free(prop_rect);
}

int CAT_room_add_prop(int item_id, CAT_ivec2 place)
{
	if(room.prop_count >= CAT_GRID_SIZE)
		return -1;
	CAT_item* prop = CAT_item_get(item_id);
	if(prop == NULL)
		return -1;

	CAT_rect prop_rect = CAT_rect_place(place, prop->data.prop_data.shape);
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
		CAT_item_list_add(&bag, room.prop_children[idx], 1);

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

	if(prop->data.prop_data.animate || prop->sprite->frame_count == 1)
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

int CAT_spawn_pickup(CAT_vec2 origin, CAT_vec2 place, const CAT_sprite* sprite, void (*proc)())
{
	if(room.pickup_count >= CAT_MAX_PICKUP_COUNT)
		return -1;
	
	int idx = room.pickup_count;
	room.pickup_count += 1;
	room.pickups[idx].origin = origin;
	room.pickups[idx].place = place;
	room.pickups[idx].sprite = sprite;
	room.pickups[idx].proc = proc;
	room.pickups[idx].timer_id = CAT_timer_init(0.75f);

	return idx;
}

void CAT_despawn_pickup(int idx)
{
	if(idx < 0 || idx >= room.pickup_count)
		return;
	
	CAT_timer_delete(room.pickups[idx].timer_id);
	for(int i = idx; i < room.pickup_count-1; i++)
		room.pickups[i] = room.pickups[i+1];
	room.pickup_count -=1;
}

void earn_proc()
{
	coins += 1;
}

void CAT_room_earn(int ticks)
{
	for(int i = 0; i < room.prop_count; i++)
	{
		if(room.prop_ids[i] == prop_eth_farm_item)
		{
			for(int t = 0; t < ticks; t++)
			{
				CAT_ivec2 start = CAT_grid2world(room.prop_rects[i].min);
				start.x += 24;
				start.y -= 24;

				CAT_ivec2 end_grid = CAT_rand_free_space();
				CAT_ivec2 end_world = CAT_grid2world(end_grid);

				float xi = start.x;
				float yi = start.y;
				float xf = end_world.x;
				float yf = end_world.y;

				CAT_spawn_pickup
				(
					(CAT_vec2) {xi, yi},
					(CAT_vec2) {xf, yf},
					&coin_world_sprite,
					earn_proc
				);
			}
		}
	}
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
	if(CAT_timer_tick(room.earn_timer_id))
	{
		CAT_room_earn(1);
		CAT_timer_reset(room.earn_timer_id);
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

	if(CAT_get_machine_state() != CAT_MS_room)
		return;

	for(int i = 0; i < 5; i++)
	{
		if(CAT_input_touch(8+16+48*i, 280+16, 16))
		{
			CAT_machine_transition(button_modes[i]);
			mode_selector = i;
		}
	}

	if(CAT_input_touch_rect(176, 16, 56, 96))
		CAT_machine_transition(CAT_MS_vending);
	if(CAT_input_touch_rect(128, 48, 32, 64))
		CAT_machine_transition(CAT_MS_arcade);

	for(int i = 0; i < room.pickup_count; i++)
	{
		if(CAT_timer_tick(room.pickups[i].timer_id))
		{
			CAT_vec2 place = room.pickups[i].place;
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
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_menu);

			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
			{
				mode_selector += 1;
				if(mode_selector > 4)
					mode_selector = 0;
			}
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
			{
				mode_selector -= 1;
				if(mode_selector < 0)
					mode_selector = 4;
			}
			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_machine_transition(button_modes[mode_selector]);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

enum {WALL_BASE, WALL_SKY} bg_wall = WALL_BASE;
enum {FLOOR_BASE, FLOOR_GRASS, FLOOR_ASH} bg_floor = FLOOR_BASE;

void render_background()
{
	draw_mode = CAT_DRAW_MODE_DEFAULT;
	for(int y = 0; y < 6; y++)
	{
		for(int x = 0; x < 15; x++)
		{
			int tile_idx = room.theme->wall_map[y * 15 + x];
			CAT_draw_sprite(room.theme->wall_tiles, tile_idx, x * 16, y * 16);
		}
	}
	for(int y = 0; y < 14; y++)
	{
		for(int x = 0; x < 15; x++)
		{
			int tile_idx = room.theme->floor_map[y * 15 + x];
			CAT_draw_sprite(room.theme->floor_tiles, tile_idx, x * 16, (y + 6) * 16);
		}
	}
}

float battery_blink_timer = 0.0f;
bool battery_blink_switch = false;
void render_statics()
{	
	CAT_datetime time;
	CAT_get_datetime(&time);
	float aqi_score = CAT_AQI_aggregate();

	if(aqi_score <= 35.0f && time.hour >= 4 && time.hour < 22)
		CAT_draw_queue_add(&window_day_bad_aq_sprite, -1, STATICS_LAYER, 8, 8, CAT_DRAW_MODE_DEFAULT);
	else if(time.hour >= 4 && time.hour < 7)
		CAT_draw_queue_add(&window_dawn_sprite, 0, STATICS_LAYER, 8, 8, CAT_DRAW_MODE_DEFAULT);
	else if(time.hour >= 7 && time.hour < 11)
		CAT_draw_queue_add(&window_morning_sprite, 0, STATICS_LAYER, 8, 8, CAT_DRAW_MODE_DEFAULT);
	else if(time.hour >= 11 && time.hour < 18)
		CAT_draw_queue_add(&window_day_sprite, 0, STATICS_LAYER, 8, 8, CAT_DRAW_MODE_DEFAULT);
	else if(time.hour >= 18 && time.hour < 20)
		CAT_draw_queue_add(&window_evening_sprite, 0, STATICS_LAYER, 8, 8, CAT_DRAW_MODE_DEFAULT);
	else if(time.hour >= 20 && time.hour < 22)
		CAT_draw_queue_add(&window_dusk_sprite, 0, STATICS_LAYER, 8, 8, CAT_DRAW_MODE_DEFAULT);
	else
		CAT_draw_queue_add(&window_night_sprite, 0, STATICS_LAYER, 8, 8, CAT_DRAW_MODE_DEFAULT);
	
	
	if(CAT_is_charging())
	{
		CAT_draw_queue_add(&icon_charging, 0, STATICS_LAYER + 1, 66, 37, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y);
	}
	else
	{
		battery_blink_timer += CAT_get_delta_time();
		if(battery_blink_timer >= 0.5f)
		{
			battery_blink_timer = 0;
			battery_blink_switch = !battery_blink_switch;
		}
		if(CAT_get_battery_pct() <= CAT_CRITICAL_BATTERY_PCT && battery_blink_switch)
			CAT_draw_queue_add(&icon_low_battery_alt, 0, STATICS_LAYER + 1, 66, 37, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y);
	}
	
	CAT_draw_queue_add(&vending_sprite, -1, STATICS_LAYER, 172, 16, CAT_DRAW_MODE_DEFAULT);
	CAT_draw_queue_add(&arcade_sprite, -1, STATICS_LAYER, 124, 48, CAT_DRAW_MODE_DEFAULT);
}

void render_props()
{
	for(int i = 0; i < room.prop_count; i++)
	{
		int prop_id = room.prop_ids[i];
		CAT_item* prop = CAT_item_get(prop_id);
		CAT_ivec2 shape = prop->data.prop_data.shape;
		CAT_ivec2 place = room.prop_rects[i].min;

		CAT_ivec2 draw_place = CAT_grid2world(place);
		draw_place.y += shape.y * CAT_TILE_SIZE;

		int mode = CAT_DRAW_MODE_BOTTOM;
		int frame_idx = 0;
		if(room.prop_overrides[i])
		{
			if(prop->data.prop_data.animate || prop->sprite->frame_count == 1)
				mode |= CAT_DRAW_MODE_REFLECT_X;
			else
				frame_idx = room.prop_overrides[i];
		}
		int job = prop->data.prop_data.animate ?
		CAT_draw_queue_add(prop->sprite, -1, PROPS_LAYER, draw_place.x, draw_place.y, mode) :
		CAT_draw_queue_add(prop->sprite, frame_idx, PROPS_LAYER, draw_place.x, draw_place.y, mode);
		
		CAT_item* child = CAT_item_get(room.prop_children[i]);
		if(child != NULL)
		{
			mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;
			frame_idx = child->data.prop_data.animate ? -1 : 0;
			int cx = (shape.x / 2) * CAT_TILE_SIZE;
			int cy = shape.y * CAT_TILE_SIZE;
			int dy = child->data.prop_data.child_dy;
			CAT_draw_queue_insert(job+1, child->sprite, frame_idx, PROPS_LAYER, draw_place.x + cx, draw_place.y - cy - dy, mode);
		}
	}
}

void render_pickups()
{
	for(int i = 0; i < room.pickup_count; i++)
	{
		CAT_vec2 origin = room.pickups[i].origin;
		CAT_vec2 place = room.pickups[i].place;

		float t = CAT_timer_progress(room.pickups[i].timer_id);
		float x = lerp(origin.x, place.x, t);
		if(origin.y > place.y)
			t = 1.0f - t;
		float y = lerp(origin.y, place.y, 3*t*t - 2*t);
		if(origin.y > place.y)
			y = -y + origin.y + place.y;

		CAT_draw_queue_add(room.pickups[i].sprite, -1, PROPS_LAYER, x, y, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_BOTTOM);
	}
}

void render_pet()
{
	CAT_anim_tick(&AM_pet);
	CAT_anim_tick(&AM_mood);
	
	int mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;
	if(pet.rot != 0)
		mode |= CAT_DRAW_MODE_REFLECT_X;
	int layer = CAT_get_battery_pct() <= CAT_CRITICAL_BATTERY_PCT ? 1 : 2;

	CAT_draw_queue_add(CAT_anim_read(&AM_pet), -1, layer, pet.pos.x, pet.pos.y, mode);

	if(CAT_anim_is_in(&AM_mood, &AS_react))
	{
		int x_off = pet.rot != 0 ? 16 : -16;
		CAT_draw_queue_add(CAT_anim_read(&AM_mood), -1, layer+1, pet.pos.x + x_off, pet.pos.y - 48, mode);	
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
static int button_start_x = 24;
static int button_start_y = 296;

void render_gui()
{
	int icon_mode = CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y;
	for(int i = 0; i < sizeof(button_sprites)/sizeof(button_sprites[0]); i++)
	{
		CAT_draw_queue_add(button_sprites[i], 0, GUI_LAYER, button_start_x + 48 * i, button_start_y, icon_mode);
	}
	CAT_draw_queue_add(&button_hl_sprite, 0, GUI_LAYER, button_start_x + 48 * mode_selector, button_start_y, icon_mode);

	if(input.touch.pressure)
		CAT_draw_queue_add(&touch_hl_sprite, 0, 4, input.touch.x, input.touch.y, icon_mode);
}

void CAT_render_room()
{
	render_background();
	render_statics();
	render_props();
	render_pickups();
	render_pet();
	render_gui();
}
