#include "cat_deco.h"

#include "cat_item.h"
#include "cat_room.h"
#include "cat_machine.h"
#include "cat_input.h"
#include "cat_pet.h"
#include "cat_render.h"
#include <stdio.h>
#include "cat_item.h"
#include "cat_menu.h"
#include "sprite_assets.h"
#include "cat_gui.h"
#include "cat_gizmos.h"

static enum {ADD, MOD, REMOVE} mode = ADD;
static bool selecting = false;

static int cursor_x;
static int cursor_y;
static int hover_idx;

static CAT_prop_list* props;
static int hold_id = -1;

static void get_hover_rect(int* x0, int* y0, int* x1, int* y1)
{
	if(hover_idx != -1)
	{
		*x0 = props->data[hover_idx].x0;
		*y0 = props->data[hover_idx].y0;
		*x1 = props->data[hover_idx].x1;
		*y1 = props->data[hover_idx].y1;
	}
}

static void get_hold_rect(int* x0, int* y0, int* x1, int* y1)
{
	if(hold_id != -1)
	{
		CAT_item* item = CAT_get_item(hold_id);
		*x0 = cursor_x;
		*y0 = cursor_y;
		*x1 = cursor_x + item->prop_shape.x;
		*y1 = cursor_y + item->prop_shape.y;
	}
}

static bool can_place_held()
{
	if(hold_id == -1)
		return false;
	int x0, y0, x1, y1;
	get_hold_rect(&x0, &y0, &x1, &y1);
	return CAT_room_is_block_free(x0, y0, x1-x0, y1-y0);
}

static bool can_stack_held()
{
	if(hold_id != -1 && hover_idx != -1 && props->data[hover_idx].child == -1)
	{
		CAT_item* hold_item = CAT_get_item(hold_id);
		int hold_x0, hold_y0, hold_x1, hold_y1;
		get_hold_rect(&hold_x0, &hold_y0, &hold_x1, &hold_y1);
		
		CAT_item* hover_item = CAT_get_item(props->data[hover_idx].prop);
		int hover_x0, hover_y0, hover_x1, hover_y1;
		get_hold_rect(&hover_x0, &hover_y0, &hover_x1, &hover_y1);

		return
		hover_item->prop_type == CAT_PROP_TYPE_BOTTOM &&
		hold_item->prop_type == CAT_PROP_TYPE_TOP &&
		CAT_rect_contains_rect
		(
			hover_x0, hover_y0,
			hover_x1, hover_y1,
			hold_x0, hold_y0,
			hold_x1, hold_y1
		);
	}
	return false;
}

static void select_proc(int item_id)
{
	hold_id = item_id;
	selecting = false;
}

void CAT_MS_deco(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_deco);
			CAT_pet_settle();
			
			mode = ADD;
			selecting = false;

			CAT_room_nearest_free_cell(4, 3, &cursor_x, &cursor_y);
			
			props = CAT_room_get_props();
			hold_id = -1;

			CAT_gui_begin_item_grid_context(false);
			break;
		}
		case CAT_FSM_SIGNAL_TICK:
		{
			if(selecting)
			{
				CAT_gui_begin_item_grid();
				CAT_gui_item_grid_add_tab("", NULL, select_proc);

				for(int i = 0; i < item_table.length; i++)
				{
					if(item_table.counts[i] <= 0)
						continue;
					if(item_table.data[i].type != CAT_ITEM_TYPE_PROP)
						continue;
					CAT_gui_item_grid_cell(i);
				}

				if(CAT_input_pressed(CAT_BUTTON_B))
					selecting = false;

				return;
			}
			else
			{	
				if(CAT_input_pressed(CAT_BUTTON_B) && hold_id == -1)
					CAT_pushdown_pop();
			
				if(CAT_input_pressed(CAT_BUTTON_SELECT))
				{
					mode += 1;
					if(mode > REMOVE)
						mode = ADD;
				}

				if(CAT_input_pulse(CAT_BUTTON_UP))
					cursor_y -= 1;
				if(CAT_input_pulse(CAT_BUTTON_RIGHT))
					cursor_x += 1;
				if(CAT_input_pulse(CAT_BUTTON_DOWN))
					cursor_y += 1;
				if(CAT_input_pulse(CAT_BUTTON_LEFT))
					cursor_x -= 1;

				int col_w = 1;
				int col_y = 1;
				if(hold_id != -1)
				{
					CAT_item* item = CAT_get_item(hold_id);
					col_w = item->prop_shape.x;
					col_y = item->prop_shape.y;
				}

				cursor_x = CAT_clamp(cursor_x, 0, CAT_ROOM_GRID_W-col_w);
				cursor_y = CAT_clamp(cursor_y, 0, CAT_ROOM_GRID_H-col_y);

				hover_idx = CAT_room_cell_lookup(cursor_x, cursor_y);
			}

			switch(mode)
			{
				case ADD:
				{
					if(hold_id != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							if(can_place_held())
							{
								CAT_room_place_prop(cursor_x, cursor_y, hold_id);
								CAT_inventory_remove(hold_id, 1);
								hold_id = -1;
								hover_idx = CAT_room_cell_lookup(cursor_x, cursor_y);
							}
							else if(can_stack_held())
							{
								CAT_room_stack_prop(hover_idx, hold_id);
								CAT_inventory_remove(hold_id, 1);
								hold_id = -1;
							}
						}
						else if(CAT_input_pressed(CAT_BUTTON_B))
						{
							hold_id = -1;
						}
					}
					else if(hover_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							cursor_x = props->data[hover_idx].x0;
							cursor_y = props->data[hover_idx].y0;
							
							int prop_id = props->data[hover_idx].prop;
							int child_id = props->data[hover_idx].child;

							if(child_id != -1)
							{
								hold_id = child_id;
								CAT_inventory_add(child_id, 1);
								CAT_room_unstack_prop(hover_idx);
							}
							else
							{
								hold_id = prop_id;
								CAT_inventory_add(prop_id, 1);
								CAT_room_remove_prop(hover_idx);
							}
						}
					}
					else
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
							selecting = true;
					}
					break;
				}
				case MOD:
				{
					if(hover_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
							CAT_room_alter_prop(hover_idx);
					}
					break;
				}
				case REMOVE:
				{
					if(hover_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							int prop_id = props->data[hover_idx].prop;
							int child_id = props->data[hover_idx].child;

							if(child_id == -1)
							{
								CAT_inventory_add(prop_id, 1);
								CAT_room_remove_prop(hover_idx);
							}
							else
							{
								CAT_inventory_add(child_id, 1);
								CAT_room_unstack_prop(hover_idx);
							}
						}
					}
					break;
				}
			}		

			break;
		}
		case CAT_FSM_SIGNAL_EXIT:
		{
			hold_id = -1;
			hover_idx = -1;
			break;
		}
	}
}

void CAT_render_deco()
{
	CAT_room_draw_statics();
	CAT_draw_corner_box(CAT_ROOM_X, CAT_ROOM_Y, CAT_ROOM_MAX_X, CAT_ROOM_MAX_Y, CAT_WHITE);
	CAT_draw_dot_grid(CAT_ROOM_X, CAT_ROOM_Y, CAT_ROOM_GRID_W, CAT_ROOM_GRID_H, CAT_TILE_SIZE, CAT_RGB8882565(200,200,200));

	if (CAT_get_render_cycle() == 0)
		CAT_draw_queue_clear();
	CAT_room_draw_props();
	CAT_room_draw_pickups();
	CAT_room_draw_pet();

	if(mode == ADD)
	{
		if(hold_id != -1)
		{
			const CAT_sprite* tile_sprite = (can_place_held() || can_stack_held()) ? &tile_hl_sprite : &tile_hl_rm_sprite;
			int x0, y0, x1, y1;
			int draw_x, draw_y;
			get_hold_rect(&x0, &y0, &x1, &y1);

			for(int y = y0; y < y1; y++)
			{
				for(int x = x0; x < x1; x++)
				{
					CAT_room_cell2point(x, y, &draw_x, &draw_y);
					CAT_draw_queue_add(tile_sprite, 0, 3, draw_x, draw_y, CAT_DRAW_FLAG_NONE);
				}
			}

			CAT_item* item = CAT_get_item(hold_id);
			int prop_height = (y1-y0) * CAT_TILE_SIZE;
			CAT_room_cell2point(cursor_x, cursor_y, &draw_x, &draw_y);
			CAT_draw_queue_add(item->sprite, 0, 2, draw_x, draw_y+prop_height, CAT_DRAW_FLAG_BOTTOM);
		}
		else if(hover_idx != -1)
		{
			int x0 = props->data[hover_idx].x0;
			int y0 = props->data[hover_idx].y0;
			int x1 = props->data[hover_idx].x1;
			int y1 = props->data[hover_idx].y1;
			int draw_x, draw_y;

			for(int y = y0; y < y1; y++)
			{
				for(int x = x0; x < x1; x++)
				{
					CAT_room_cell2point(x, y, &draw_x, &draw_y);
					CAT_draw_queue_add(&tile_hl_sprite, 0, 3, draw_x, draw_y, CAT_DRAW_FLAG_NONE);
				}
			}
			
			CAT_room_cell2point(cursor_x, cursor_y, &draw_x, &draw_y);
			CAT_draw_queue_add(&tile_hl_add_sprite, 0, 3, draw_x, draw_y, CAT_DRAW_FLAG_NONE);
		}
		else
		{
			int draw_x, draw_y;
			CAT_room_cell2point(cursor_x, cursor_y, &draw_x, &draw_y);
			CAT_draw_queue_add(&cursor_add_sprite, 0, 3, draw_x, draw_y, CAT_DRAW_FLAG_NONE);
		}
	}
	else
	{
		const CAT_sprite* tile_hl = mode == MOD ? &tile_hl_flip_sprite : &tile_hl_rm_sprite;
		const CAT_sprite* tile_mark = mode == MOD ? &tile_mark_flip_sprite : &tile_mark_rm_sprite;
		const CAT_sprite* cursor_sprite = mode == MOD ? &cursor_flip_sprite : &cursor_remove_sprite;

		if(hover_idx != -1)
		{
			int x0 = props->data[hover_idx].x0;
			int y0 = props->data[hover_idx].y0;
			int x1 = props->data[hover_idx].x1;
			int y1 = props->data[hover_idx].y1;
			int draw_x, draw_y;

			for(int y = y0; y < y1; y++)
			{
				for(int x = x0; x < x1; x++)
				{
					CAT_room_cell2point(x, y, &draw_x, &draw_y);
					CAT_draw_queue_add(tile_hl, 0, 3, draw_x, draw_y, CAT_DRAW_FLAG_NONE);
				}
			}

			CAT_room_cell2point(cursor_x, cursor_y, &draw_x, &draw_y);
			CAT_draw_queue_add(tile_mark, 0, 3, draw_x, draw_y, CAT_DRAW_FLAG_NONE);
		}
		else
		{
			int draw_x, draw_y;
			CAT_room_cell2point(cursor_x, cursor_y, &draw_x, &draw_y);
			CAT_draw_queue_add(cursor_sprite, 0, 3, draw_x, draw_y, CAT_DRAW_FLAG_NONE);
		}
	}

	CAT_draw_queue_submit();
}