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
#include "sprite_assets.h"

static enum {ADD, MOD, REMOVE} mode = ADD;

static CAT_ivec2 cursor;
static int hold_id = -1;
static int hover_idx = -1;

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

static CAT_rect hold_rect()
{
	if(hold_id != -1)
	{
		CAT_item* item = CAT_item_get(hold_id);
		return CAT_rect_place(cursor, item->data.prop_data.shape);
	}
	else
	{
		return (CAT_rect) {{0, 0}, {0, 0}};
	}
}

static bool can_place()
{
	if(hold_id != -1)
		return CAT_is_block_free(hold_rect());
	return false;
}

static bool can_stack()
{
	if(hold_id != -1 && hover_idx != -1 && room.prop_children[hover_idx] == -1)
	{
		CAT_item* hold_item = CAT_item_get(hold_id);
		CAT_item* hover_item = CAT_item_get(room.prop_ids[hover_idx]);
		return
		hover_item->data.prop_data.type == CAT_PROP_TYPE_BOTTOM &&
		hold_item->data.prop_data.type == CAT_PROP_TYPE_TOP &&
		CAT_rect_contains(room.prop_rects[hover_idx], hold_rect());
	}
	return false;
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
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();
			
			if(CAT_input_pressed(CAT_BUTTON_SELECT))
			{
				mode += 1;
				if(mode > REMOVE)
					mode = ADD;
			}

			control_cursor();
			hover_idx = CAT_room_find_spatial(cursor);

			switch(mode)
			{
				case ADD:
				{
					if(hold_id != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A) || (CAT_input_pressed(CAT_BUTTON_B)))
						{
							if(can_place())
							{
								CAT_room_add_prop(hold_id, cursor);
								CAT_item_list_remove(&bag, hold_id, 1);
								hold_id = -1;
								hover_idx = CAT_room_find_spatial(cursor);
							}
							else if(can_stack())
							{
								CAT_room_stack_prop(hover_idx, hold_id);
								CAT_item_list_remove(&bag, hold_id, 1);
								hold_id = -1;
							}
						}
					}
					else if(hover_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							cursor = room.prop_rects[hover_idx].min;
							if(room.prop_children[hover_idx] != -1)
							{
								hold_id = room.prop_children[hover_idx];
								CAT_item_list_add(&bag, room.prop_children[hover_idx], 1);
								CAT_room_unstack_prop(hover_idx);
							}
							else
							{
								hold_id = room.prop_ids[hover_idx];
								CAT_item_list_add(&bag, room.prop_ids[hover_idx], 1);
								CAT_room_remove_prop(hover_idx);
							}
						}
					}
					else
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							CAT_filter_item_dialog(prop_filter);
							CAT_target_item_dialog(&hold_id, false);
							CAT_machine_transition(CAT_MS_item_dialog);
						}
					}
					break;
				}
				case MOD:
				{
					if(hover_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
							CAT_room_flip_prop(hover_idx);
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
								CAT_item_list_add(&bag, room.prop_ids[hover_idx], 1);
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

			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			hold_id = -1;
			hover_idx = -1;
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
			const CAT_sprite* tile_sprite = (can_place() || can_stack()) ? &tile_hl_sprite : &tile_hl_rm_sprite;
			for(int y = hold_rect().min.y; y < hold_rect().max.y; y++)
			{
				for(int x = hold_rect().min.x; x < hold_rect().max.x; x++)
				{
					CAT_ivec2 draw_coords = CAT_grid2world((CAT_ivec2){x, y});
					CAT_draw_queue_add(tile_sprite, 0, 3, draw_coords.x, draw_coords.y, CAT_DRAW_FLAG_DEFAULT);
				}
			}
			CAT_item* item = CAT_item_get(hold_id);
			int tile_height = hold_rect().max.y - hold_rect().min.y;
			CAT_draw_queue_add(item->sprite, 0, 2, cursor_world.x, cursor_world.y+tile_height*16, CAT_DRAW_FLAG_BOTTOM);
		}
		else if(hover_idx != -1)
		{
			for(int y = room.prop_rects[hover_idx].min.y; y < room.prop_rects[hover_idx].max.y; y++)
			{
				for(int x = room.prop_rects[hover_idx].min.x; x < room.prop_rects[hover_idx].max.x; x++)
				{
					CAT_ivec2 draw_coords = CAT_grid2world((CAT_ivec2){x, y});
					CAT_draw_queue_add(&tile_hl_sprite, 0, 3, draw_coords.x, draw_coords.y, CAT_DRAW_FLAG_DEFAULT);
				}
			}
			CAT_draw_queue_add(&tile_hl_add_sprite, 0, 3, cursor_world.x, cursor_world.y, CAT_DRAW_FLAG_DEFAULT);
		}
		else
		{
			CAT_draw_queue_add(&cursor_add_sprite, 0, 3, cursor_world.x, cursor_world.y, CAT_DRAW_FLAG_DEFAULT);
		}
	}
	else
	{
		const CAT_sprite* tile_hl = mode == MOD ? &tile_hl_flip_sprite : &tile_hl_rm_sprite;
		const CAT_sprite* tile_mark = mode == MOD ? &tile_mark_flip_sprite : &tile_mark_rm_sprite;
		const CAT_sprite* cursor_sprite = mode == MOD ? &cursor_flip_sprite : &cursor_remove_sprite;

		if(hover_idx != -1)
		{
			for(int y = room.prop_rects[hover_idx].min.y; y < room.prop_rects[hover_idx].max.y; y++)
			{
				for(int x = room.prop_rects[hover_idx].min.x; x < room.prop_rects[hover_idx].max.x; x++)
				{
					CAT_ivec2 draw_coords = CAT_grid2world((CAT_ivec2){x, y});
					CAT_draw_queue_add(tile_hl, 0, 3, draw_coords.x, draw_coords.y, CAT_DRAW_FLAG_DEFAULT);
				}
			}
			CAT_draw_queue_add(tile_mark, 0, 3, cursor_world.x, cursor_world.y, CAT_DRAW_FLAG_DEFAULT);
		}
		else
		{
			CAT_draw_queue_add(cursor_sprite, 0, 3, cursor_world.x, cursor_world.y, CAT_DRAW_FLAG_DEFAULT);
		}
	}
}