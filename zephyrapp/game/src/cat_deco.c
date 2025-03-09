#include "cat_deco.h"

#include "cat_item.h"
#include "cat_room.h"
#include "cat_machine.h"
#include "cat_input.h"
#include "cat_pet.h"
#include "cat_render.h"
#include <stdio.h>
#include "cat_item_dialog.h"
#include "cat_bag.h"
#include "cat_menu.h"

static enum {ADD, MOD, REMOVE} mode = ADD;

static CAT_ivec2 cursor;

static int hold_id = -1;
static CAT_rect hold_rect = {0};
static bool can_pick = false;
static bool can_drop = false;

static int hover_idx = -1;
static int hover_id = -1;
static CAT_rect hover_rect = {0};

static void control_cursor()
{
	if(CAT_input_pulse(CAT_BUTTON_UP))
		cursor.y -= 1;
	if(CAT_input_pulse(CAT_BUTTON_RIGHT))
		cursor.x += 1;
	if(CAT_input_pulse(CAT_BUTTON_DOWN))
		cursor.y += 1;
	if(CAT_input_pulse(CAT_BUTTON_LEFT))
		cursor.x -= 1;
	cursor.x = clamp(cursor.x, 0, CAT_GRID_WIDTH-1);
	cursor.y = clamp(cursor.y, 0, CAT_GRID_HEIGHT-1);
}

static bool prop_filter(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return false;
	return item->type == CAT_ITEM_TYPE_PROP;
}

void CAT_MS_deco(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_deco);

			CAT_pet_settle();
			
			mode = ADD;
			if(hold_id == -1)
				cursor = CAT_largest_free_space();
			cursor = CAT_nearest_free_space(cursor);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			control_cursor();

			hover_idx = CAT_room_find_spatial(cursor);
			if(hover_idx >= 0)
			{
				hover_id = room.prop_ids[hover_idx];
				hover_rect = room.prop_rects[hover_idx];
			}
			else
			{
				hover_id = -1;
				hover_rect = (CAT_rect) {0};
			}

			if(CAT_input_pressed(CAT_BUTTON_SELECT))
			{
				mode += 1;
				if(mode > REMOVE)
					mode = ADD;
			}

			switch(mode)
			{
				case ADD:
				{
					can_drop = false;

					if(hold_id != -1)
					{
						CAT_item* hold_prop = &item_table.data[hold_id];
						hold_rect = CAT_rect_place(cursor, hold_prop->data.prop_data.shape);
						
						if(CAT_is_block_free(hold_rect))
						{	
							can_drop = true;
							if(CAT_input_pressed(CAT_BUTTON_A))
							{
								CAT_room_add_prop(hold_id, cursor);
								CAT_item_list_remove(&bag, hold_id, 1);
								hold_id = -1;
							}
						}
						else
						{
							if(hover_id != -1 && room.prop_children[hover_idx] == -1)
							{
								CAT_item* hover_prop = CAT_item_get(hover_id);
								if(hover_prop->data.prop_data.type != CAT_PROP_TYPE_BOTTOM)
									break;						
								if(hold_prop->data.prop_data.type != CAT_PROP_TYPE_TOP)
									break;		
								if(!CAT_rect_contains(hover_rect, hold_rect))
									break;

								can_drop = true;
								if(CAT_input_pressed(CAT_BUTTON_A))
								{
									CAT_room_stack_prop(hover_idx, hold_id);
									CAT_item_list_remove(&bag, hold_id, 1);
									hold_id = -1;
								}
							}
						}			
					}
					else
					{
						if(hover_id == -1)
						{
							if(CAT_input_pressed(CAT_BUTTON_A))
							{
								CAT_filter_item_dialog(prop_filter);
								CAT_anchor_item_dialog(&hold_id);
								CAT_machine_transition(CAT_MS_item_dialog);
							}
						}
						else
						{
							can_pick = true;
							int hovered_child_id = room.prop_children[hover_idx];
							if(hovered_child_id == -1)
							{
								if(CAT_input_pressed(CAT_BUTTON_A))
								{
									CAT_room_remove_prop(hover_idx);
									CAT_item_list_add(&bag, hover_id, 1);
									hold_id = hover_id;
								}
							}
							else
							{
								if(CAT_input_pressed(CAT_BUTTON_A))
								{
									CAT_room_unstack_prop(hover_idx);
									CAT_item_list_add(&bag, hovered_child_id, 1);
									hold_id = hovered_child_id;
								}
							}
						}
					}
					break;
				}
				case MOD:
				{
					if(hover_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							CAT_room_flip_prop(hover_idx);
						}
					}
					break;
				}
				case REMOVE:
				{
					if(hover_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							if(room.prop_children[hover_idx] == -1)
							{
								CAT_item_list_add(&bag, hover_id, 1);
								CAT_room_remove_prop(hover_idx);
							}
							else
							{
								int child_id = room.prop_children[hover_idx];
								CAT_item_list_add(&bag, child_id, 1);
								CAT_room_unstack_prop(hover_idx);
							}
						}
					}
					break;
				}
			}
		
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_menu);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			hold_id = -1;
			hover_id = -1;
			break;
		}
	}
}

void CAT_render_deco()
{
	CAT_render_room();

	CAT_ivec2 cursor_world = CAT_grid2world(cursor);

	if(mode == ADD)
	{
		if(hold_id != -1)
		{
			const CAT_sprite* tile_sprite = can_drop ? &tile_hl_add_sprite : &tile_hl_rm_sprite;
			for(int y = hold_rect.min.y; y < hold_rect.max.y; y++)
			{
				for(int x = hold_rect.min.x; x < hold_rect.max.x; x++)
				{
					CAT_ivec2 draw_coords = CAT_grid2world((CAT_ivec2){x, y});
					CAT_draw_queue_add(tile_sprite, 0, 3, draw_coords.x, draw_coords.y, CAT_DRAW_MODE_DEFAULT);
				}
			}
		}
		else
		{
			if(can_pick)
			{
				for(int y = hover_rect.min.y; y < hover_rect.max.y; y++)
				{
					for(int x = hover_rect.min.x; x < hover_rect.max.x; x++)
					{
						CAT_ivec2 draw_coords = CAT_grid2world((CAT_ivec2){x, y});
						CAT_draw_queue_add(&tile_hl_sprite, 0, 3, draw_coords.x, draw_coords.y, CAT_DRAW_MODE_DEFAULT);
					}
				}
			}
			CAT_draw_queue_add(&cursor_add_sprite, 0, 3, cursor_world.x, cursor_world.y, CAT_DRAW_MODE_DEFAULT);
		}
	}
	else
	{
		const CAT_sprite* tile_hl = mode == MOD ? &tile_hl_flip_sprite : &tile_hl_rm_sprite;
		const CAT_sprite* tile_mark = mode == MOD ? &tile_mark_flip_sprite : &tile_mark_rm_sprite;
		const CAT_sprite* cursor_sprite = mode == MOD ? &cursor_flip_sprite : &cursor_remove_sprite;

		if(hover_idx != -1)
		{
			for(int y = hover_rect.min.y; y < hover_rect.max.y; y++)
			{
				for(int x = hover_rect.min.x; x < hover_rect.max.x; x++)
				{
					CAT_ivec2 draw_coords = CAT_grid2world((CAT_ivec2){x, y});
					CAT_draw_queue_add(tile_hl, 0, 3, cursor_world.x, cursor_world.y, CAT_DRAW_MODE_DEFAULT);
				}
			}
			CAT_draw_queue_add(tile_mark, 0, 3, cursor_world.x, cursor_world.y, CAT_DRAW_MODE_DEFAULT);
		}
		else
		{
			CAT_draw_queue_add(cursor_sprite, 0, 3, cursor_world.x, cursor_world.y, CAT_DRAW_MODE_DEFAULT);
		}
	}
}