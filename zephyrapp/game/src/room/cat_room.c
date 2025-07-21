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
#include "cat_world.h"

//////////////////////////////////////////////////////////////////////////
// THEME

static const CAT_room_theme* theme = &basic_theme;

void CAT_room_set_theme(const CAT_room_theme* _theme)
{
	theme = _theme;
}

const CAT_room_theme* CAT_room_get_theme()
{
	return theme;
}


//////////////////////////////////////////////////////////////////////////
// GRID

struct
{
	uint8_t x, y;
	int prop;
} cells[CAT_ROOM_GRID_SIZE];

int idx_queue_backing[CAT_ROOM_GRID_SIZE];
CAT_int_list idx_queue;
bool visit_mask[CAT_ROOM_GRID_SIZE];

static void init_grid()
{
	for(int y = 0; y < CAT_ROOM_GRID_H; y++)
	{
		for(int x = 0; x < CAT_ROOM_GRID_W; x++)
		{
			int idx = y * CAT_ROOM_GRID_W + x;
			cells[idx].x = x;
			cells[idx].y = y;
			cells[idx].prop = -1;
		}
	}

	CAT_ilist(&idx_queue, idx_queue_backing, CAT_ROOM_GRID_SIZE);
	for(int i = 0; i < CAT_ROOM_GRID_SIZE; i++)
		visit_mask[i] = false;
}

void CAT_room_point2cell(int x, int y, int* x_out, int* y_out)
{
	*x_out = (x - CAT_ROOM_X) / CAT_ROOM_GRID_W;
	*y_out = (y - CAT_ROOM_Y) / CAT_ROOM_GRID_H;
}

void CAT_room_cell2point(int x, int y, int* x_out, int* y_out)
{
	*x_out = (CAT_ROOM_GRID_X + x) * CAT_TILE_SIZE;
	*y_out = (CAT_ROOM_GRID_Y + y) * CAT_TILE_SIZE;
}

bool CAT_room_in_bounds(int x, int y)
{
	if(x < CAT_ROOM_X || x > CAT_ROOM_MAX_X)
		return false;
	if(y < CAT_ROOM_Y || y > CAT_ROOM_MAX_Y)
		return false;
	return true;
}

bool CAT_room_in_grid_bounds(int x, int y)
{
	if(x < 0 || x >= CAT_ROOM_GRID_W)
		return false;
	if(y < 0 || y > CAT_ROOM_GRID_H)
		return false;
	return true;
}

bool CAT_room_is_point_free(int x, int y)
{
	if(!CAT_room_in_bounds(x, y))
		return false;
	int cell_x, cell_y;
	CAT_room_point2cell(x, y, &cell_x, &cell_y);
	return cells[cell_y * CAT_ROOM_GRID_W + cell_x].prop == -1;
}

bool CAT_room_is_cell_free(int x, int y)
{
	if(!CAT_room_in_grid_bounds(x, y))
		return false;
	return cells[y * CAT_ROOM_GRID_W + x].prop == -1;
}

bool CAT_room_is_block_free(int x, int y, int w, int h)
{
	int xf = x + w;
	int yf = y + h;
	if(x < 0 || xf > CAT_ROOM_GRID_W)
		return false;
	if(y < 0 || yf > CAT_ROOM_GRID_H)
		return false;

	for(int yc = y; yc < yf; yc++)
	{
		for(int xc = x; xc < xf; xc++)
		{
			if(cells[yc * CAT_ROOM_GRID_W + xc].prop != -1)
				return false;
		}
	}

	return true;
}

void CAT_room_set_block(int x, int y, int w, int h, int item_id)
{
	for(int dy = 0; dy < h; dy++)
	{
		for(int dx = 0; dx < w; dx++)
		{
			cells[(y+dy) * CAT_ROOM_GRID_W + (x+dx)].prop = item_id;
		}
	}
}

bool CAT_room_has_free_cell()
{
	for(int i = 0; i < CAT_ROOM_GRID_SIZE; i++)
	{
		if(cells[i].prop == -1)
			return true;
	}
	return false;
}

void CAT_room_nearest_free_cell(int x, int y, int* x_out, int* y_out)
{
	CAT_ilist_clear(&idx_queue);
	int start = y * CAT_ROOM_GRID_W + x;
	CAT_ilist_push(&idx_queue, start);

	for(int i = 0; i < CAT_ROOM_GRID_SIZE; i++)
		visit_mask[i] = false;

	while(idx_queue.length != 0)
	{
		int idx = CAT_ilist_dequeue(&idx_queue);
		if(idx < 0 || idx >= CAT_ROOM_GRID_SIZE)
			continue;
		if(visit_mask[idx])
			continue;

		if(cells[idx].prop == -1)
		{
			*x_out = cells[idx].x;
			*y_out = cells[idx].y;
			return;
		}

		visit_mask[idx] = true;		

		int N = (cells[idx].y-1) * CAT_ROOM_GRID_W + cells[idx].x;
		int E = cells[idx].y * CAT_ROOM_GRID_W + (cells[idx].x+1);
		int S = (cells[idx].y+1) * CAT_ROOM_GRID_W + cells[idx].x;
		int W = cells[idx].y * CAT_ROOM_GRID_W + (cells[idx].x-1);
		CAT_ilist_push(&idx_queue, N);
		CAT_ilist_push(&idx_queue, E);
		CAT_ilist_push(&idx_queue, S);
		CAT_ilist_push(&idx_queue, W);
	}

	*x_out = -1;
	*y_out = -1;
	return;
}

void CAT_room_random_free_cell(int* x_out, int* y_out)
{
	CAT_ilist_clear(&idx_queue);
	for(int i = 0; i < CAT_ROOM_GRID_SIZE; i++)
	{
		if(cells[i].prop == -1)
			CAT_ilist_push(&idx_queue, i);
	}
	int idx = idx_queue.data[CAT_rand_int(0, idx_queue.length-1)];
	*x_out = cells[idx].x;
	*y_out = cells[idx].y;
}


//////////////////////////////////////////////////////////////////////////
// PROPS

static CAT_prop_list prop_list =
{
	.length = 0
};

CAT_prop_list* CAT_room_get_props()
{
	return &prop_list;
}

int CAT_room_prop_lookup(int item_id)
{
	for(int i = 0; i < prop_list.length; i++)
	{
		if(prop_list.data[i].prop == item_id)
			return i;
	}
	return -1;
}

int CAT_room_cell_lookup(int x, int y)
{
	if(!CAT_room_in_grid_bounds(x, y))
		return -1;
	for(int i = 0; i < prop_list.length; i++)
	{
		int x0 = prop_list.data[i].x0;
		int y0 = prop_list.data[i].y0;
		int x1 = prop_list.data[i].x1;
		int y1 = prop_list.data[i].y1;
		if(CAT_rect_point_intersect(x0, y0, x1-1, y1-1, x, y))
			return i;
	}
	return -1;
}

bool CAT_room_fits_prop(int x, int y, int item_id)
{
	if(prop_list.length >= CAT_ROOM_GRID_SIZE)
		return false;

	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return false;
	if(item->type != CAT_ITEM_TYPE_PROP)
		return false;
	
	int w = item->prop_shape.x;
	int h = item->prop_shape.y;
	return CAT_room_is_block_free(x, y, w, h);
}

int CAT_room_place_prop(int x, int y, int item_id)
{
	if(!CAT_room_fits_prop(x, y, item_id))
		return -1;
	
	CAT_item* item = CAT_item_get(item_id);
	int w = item->prop_shape.x;
	int h = item->prop_shape.y;
	CAT_room_set_block(x, y, w, h, item_id);
	
	int idx = prop_list.length;
	prop_list.data[idx] = (struct prop_list_item)
	{
		.prop = item_id,
		.x0 = x, .y0 = y,
		.x1 = x+w, .y1 = y+h,
		.override = 0,
		.child = -1
	};
	prop_list.length += 1;

	return idx;
}

int CAT_room_remove_prop(int idx)
{
	if(idx < 0 || idx >= prop_list.length)
		return -1;

	int item_id = prop_list.data[idx].prop;
	int x = prop_list.data[idx].x0;
	int y = prop_list.data[idx].y0;
	int w = prop_list.data[idx].x1 - x;
	int h = prop_list.data[idx].y1 - y;
	CAT_room_set_block(x, y, w, h, -1);

	prop_list.length -= 1;
	for(int i = idx; i < prop_list.length; i++)
		prop_list.data[i] = prop_list.data[i+1];

	return item_id;
}

bool CAT_room_prop_has_child(int idx)
{
	if(idx < 0 || idx >= prop_list.length)
		return false;
	if(prop_list.data[idx].child == -1)
		return false;
	return true;
}

bool CAT_room_stack_prop(int idx, int item_id)
{
	if(idx < 0 || idx >= prop_list.length)
		return false;
	if(CAT_room_prop_has_child(idx))
		return false;

	int parent_id = prop_list.data[idx].prop;
	CAT_item* parent = CAT_item_get(parent_id);
	if(parent == NULL)
		return false;
	if(parent->type != CAT_ITEM_TYPE_PROP || parent->prop_type != CAT_PROP_TYPE_BOTTOM)
		return false;
	
	CAT_item* child = CAT_item_get(item_id);
	if(child == NULL)
		return false;
	if(child->type != CAT_ITEM_TYPE_PROP || child->prop_type != CAT_PROP_TYPE_TOP)
		return false;

	prop_list.data[idx].child = item_id;
	return true;
}

int CAT_room_unstack_prop(int idx)
{
	if(idx < 0 || idx >= prop_list.length)
		return -1;
	int child_id = prop_list.data[idx].child;
	prop_list.data[idx].child = -1;
	return child_id;
}

int CAT_room_alter_prop(int idx)
{
	if(idx < 0 || idx >= prop_list.length)
		return -1;
	int item_id = prop_list.data[idx].prop;

	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return -1;
	if(item->type != CAT_ITEM_TYPE_PROP || item->prop_animated)
		return -1;

	int variations = item->sprite->frame_count;
	prop_list.data[idx].override = wrap(prop_list.data[idx].override+1, variations);
	return prop_list.data[idx].override;
}


//////////////////////////////////////////////////////////////////////////
// PICKUPS

static CAT_pickup_list pickup_list =
{
	.length = 0
};

CAT_pickup_list* CAT_room_get_pickups()
{
	return &pickup_list;
}

int CAT_room_spawn_pickup(CAT_pickup pickup)
{
	if(pickup_list.length >= CAT_ROOM_MAX_PICKUP_COUNT)
		return -1;

	int idx = pickup_list.length;
	pickup_list.data[idx] = pickup;
	return idx;
}

void CAT_room_consume_pickup(int idx)
{
	if(idx < 0 || idx >= pickup_list.length)
		return;
	
	pickup_list.data[idx].proc();
	pickup_list.length -= 1;
	for(int i = idx; i < pickup_list.length; i++)
		pickup_list.data[i] = pickup_list.data[i+1];
}


//////////////////////////////////////////////////////////////////////////
// ROOM

static CAT_datetime datetime;

void CAT_room_init()
{
	theme = &basic_theme;
	init_grid();
	prop_list.length = 0;
	pickup_list.length = 0;
}

#define PICKUP_AIR_TIME 0.75f

void CAT_room_tick()
{
	for(int i = 0; i < pickup_list.length; i++)
	{
		if(pickup_list.data[i].timer < PICKUP_AIR_TIME)
			pickup_list.data[i].timer += CAT_get_delta_time_s();
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

void screen_button_input()
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

void prop_button_input()
{
	if(CAT_input_touch_rect(VENDING_MACHINE_X, VENDING_MACHINE_Y, VENDING_MACHINE_W, VENDING_MACHINE_H))
		CAT_machine_transition(CAT_MS_shop);
	else if(CAT_input_touch_rect(ARCADE_X, ARCADE_Y, ARCADE_W, ARCADE_H))
		CAT_machine_transition(CAT_MS_arcade);
	else if(CAT_input_touch_rect
	(
		theme->window_rect.min.x+4,
		theme->window_rect.min.y+4,
		theme->window_rect.max.x-theme->window_rect.min.x-8,
		theme->window_rect.max.y-theme->window_rect.min.y-8
	))
	{
		CAT_machine_transition(CAT_MS_monitor);
	}
}

void pickup_input()
{
	for(int i = 0; i < pickup_list.length; i++)
	{
		if(pickup_list.data[i].timer >= PICKUP_AIR_TIME)
		{
			int x = pickup_list.data[i].x1;
			int y = pickup_list.data[i].y1;
			if(CAT_input_drag(x+8, y-8, 16))
			{
				CAT_room_consume_pickup(i);
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

			if(CAT_input_spell(basic_spell))
			{
				CAT_machine_transition(CAT_MS_world);
				break;
			}
				
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_menu);

			if(CAT_pet_is_dead())
			{
				if(CAT_input_touch_rect(120-32, 228-80, 64, 80))
				{
					CAT_pet_reincarnate();
				}

				prop_button_input();
				pickup_input();
			}
			else
			{
				screen_button_input();
				prop_button_input();
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
	int window_y = theme->window_rect.min.y;
	int window_x = theme->window_rect.min.x;
	int window_width = theme->window_rect.max.x-theme->window_rect.min.x;
	int window_height = theme->window_rect.max.y-theme->window_rect.min.y;
	int window_columns = window_width/8;
	int sky_row_off = datetime.hour * (480 - window_height) / 23;

	if(theme->tile_wall)
	{
		for(int y = 0; y < 6; y++)
		{
			for(int x = 0; x < 15; x++)
			{
				int tile_idx = theme->wall_map[y * 15 + x];
				CAT_draw_tile(theme->wall_tiles, tile_idx, x * 16, y * 16);
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
		CAT_draw_background(theme->wall_tiles, 0, 0);
	}

	if(theme->tile_floor)
	{
		for(int y = 0; y < 14; y++)
		{
			for(int x = 0; x < 15; x++)
			{
				int tile_idx = theme->floor_map[y * 15 + x];
				CAT_draw_tile(theme->floor_tiles, tile_idx, x * 16, (6 + y) * 16);
			}
		}
	}
	else
	{
		int row_offset = theme->tile_wall ?
		16*16 : theme->wall_tiles->height;
		CAT_draw_background(theme->floor_tiles, 0, row_offset);
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
	for(int i = 0; i < prop_list.length; i++)
	{
		int prop_id = prop_list.data[i].prop;
		CAT_item* prop = CAT_item_get(prop_id);
		int x0 = prop_list.data[i].x0;
		int y0 = prop_list.data[i].y0;
		int x1 = prop_list.data[i].x1;
		int y1 = prop_list.data[i].y1;

		int draw_x, draw_y;
		CAT_room_cell2point(x0, y0, &draw_x, &draw_y);
		draw_y += (y1-y0) * CAT_TILE_SIZE;

		int flags = CAT_DRAW_FLAG_BOTTOM;
		int frame_idx = 0;
		if(prop_list.data[i].override > 0)
		{
			if(prop->prop_animated || prop->sprite->frame_count == 1)
				flags |= CAT_DRAW_FLAG_REFLECT_X;
			else
				frame_idx = prop_list.data[i].override;
		}
		int job = prop->prop_animated ?
		CAT_draw_queue_add(prop->sprite, -1, PROPS_LAYER, draw_x, draw_y, flags) :
		CAT_draw_queue_add(prop->sprite, frame_idx, PROPS_LAYER, draw_x, draw_y, flags);
		
		CAT_item* child = CAT_item_get(prop_list.data[i].child);
		if(child != NULL)
		{
			flags = CAT_DRAW_FLAG_BOTTOM | CAT_DRAW_FLAG_CENTER_X;
			frame_idx = child->prop_animated ? -1 : 0;
			int cx = (x1-x0)/2 * CAT_TILE_SIZE;
			int cy = (y1-y0) * CAT_TILE_SIZE + child->prop_child_dy;
			CAT_draw_queue_insert(job+1, child->sprite, frame_idx, PROPS_LAYER, draw_x + cx, draw_y - cy, flags);
		}
	}
}

void CAT_room_draw_pickups()
{
	for(int i = 0; i < pickup_list.length; i++)
	{
		int x0 = pickup_list.data[i].x0;
		int y0 = pickup_list.data[i].y0;
		int x1 = pickup_list.data[i].x1;
		int y1 = pickup_list.data[i].y1;

		float t = min(pickup_list.data[i].timer / PICKUP_AIR_TIME, 1.0f);
		float x = lerp(x0, x1, t);
		if(y0 > y1)
			t = 1.0f - t;
		float y = lerp(y0, y1, 3*t*t - 2*t);
		if(y0 > y1)
			y = -y + y0 + y1;

		CAT_draw_queue_add(pickup_list.data[i].sprite, -1, PROPS_LAYER, x, y, CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_BOTTOM);
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
