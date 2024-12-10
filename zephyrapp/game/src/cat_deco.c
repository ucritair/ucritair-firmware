#include "cat_deco.h"

#include "cat_item.h"
#include "cat_room.h"
#include "cat_machine.h"
#include "cat_input.h"
#include "cat_pet.h"
#include "cat_sprite.h"
#include <stdio.h>
#include "cat_item_dialog.h"
#include "cat_bag.h"
#include "cat_menu.h"

CAT_deco_state deco_state =
{
	.mode = ADD,

	.add_id = -1,
	.add_rect = {0},
	.valid_add = false,

	.mod_idx = -1,
	.mod_rect = {0}
};

void CAT_deco_target(CAT_ivec2 place)
{
	for(int i = 0; i < room.prop_count; i++)
	{
		int item_id = room.prop_ids[i];
		CAT_item* item = &item_table.data[item_id];
		CAT_ivec2 shape = item->data.prop_data.shape;
		CAT_rect block = CAT_rect_place(room.prop_places[i], shape);

		if(place.x < block.min.x || place.x >= block.max.x)
			continue;
		if(place.y < block.min.y || place.y >= block.max.y)
			continue;
		
		deco_state.mod_idx = i;
		deco_state.mod_rect = block;
		return;
	}

	deco_state.mod_idx = -1;
	deco_state.mod_rect = (CAT_rect) {{0, 0}, {0, 0}};
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
			if(deco_state.add_id == -1)
				room.grid_cursor = CAT_largest_free_space();
			room.grid_cursor = CAT_nearest_free_space(room.grid_cursor);
			
			CAT_pet_settle();

			deco_state.mode = ADD;
			deco_state.mod_idx = -1;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_room_cursor();

			if(CAT_input_pressed(CAT_BUTTON_SELECT))
			{
				deco_state.mode += 1;
				if(deco_state.mode > REMOVE)
					deco_state.mode = ADD;
				deco_state.add_id = -1;
			}

			switch(deco_state.mode)
			{
				case ADD:
				{
					if(deco_state.add_id != -1)
					{
						CAT_item* prop = &item_table.data[deco_state.add_id];
						CAT_ivec2 shape = prop->data.prop_data.shape;
						CAT_rect block = CAT_rect_place(room.grid_cursor, shape);

						deco_state.add_rect = block;
						deco_state.valid_add = false;
						
						if(CAT_is_block_free(block))
						{
							if(CAT_input_pressed(CAT_BUTTON_A))
							{
								CAT_room_add_prop(deco_state.add_id, room.grid_cursor);
								CAT_item_list_remove(&bag, deco_state.add_id, 1);
								deco_state.add_id = -1;
							}
							deco_state.valid_add = true;
						}
						else
						{
							int base_idx = CAT_room_find_spatial(room.grid_cursor);
							if(base_idx != -1)
							{
								int base_id = room.prop_ids[base_idx];
								CAT_item* base_prop = CAT_item_get(base_id);
								bool bot_compat = base_prop->data.prop_data.type == CAT_PROP_TYPE_BOTTOM;
								bool top_compat = prop->data.prop_data.type == CAT_PROP_TYPE_TOP;
								bool space_compat =
								base_prop->data.prop_data.shape.x >= prop->data.prop_data.shape.x &&
								base_prop->data.prop_data.shape.y >= prop->data.prop_data.shape.y;
								if(bot_compat && top_compat && space_compat)
								{
									if(CAT_input_pressed(CAT_BUTTON_A))
									{
										if(room.prop_children[base_idx] == -1)
										{
											CAT_room_stack_prop(base_idx, deco_state.add_id);
											CAT_item_list_remove(&bag, deco_state.add_id, 1);
										}
										else
										{
											CAT_item_list_add(&bag, room.prop_children[base_idx], 1);
											CAT_room_unstack_prop(base_idx);
										}
										deco_state.add_id = -1;
									}
									deco_state.valid_add = true;
								}
							}
						}				
					}
					else
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							CAT_filter_item_dialog(prop_filter);
							CAT_anchor_item_dialog(&deco_state.add_id);
							CAT_machine_transition(CAT_MS_item_dialog);
						}
					}
					break;
				}
				case FLIP:
				{
					CAT_deco_target(room.grid_cursor);
					if(deco_state.mod_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							CAT_room_flip_prop(deco_state.mod_idx);
						}
					}
					break;
				}
				case REMOVE:
				{
					CAT_deco_target(room.grid_cursor);
					if(deco_state.mod_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							if(room.prop_children[deco_state.mod_idx] == -1)
							{
								int item_id = room.prop_ids[deco_state.mod_idx];
								CAT_item_list_add(&bag, item_id, 1);
								CAT_room_remove_prop(deco_state.mod_idx);
								deco_state.mod_idx = -1;
							}
							else
							{
								int item_id = room.prop_children[deco_state.mod_idx];
								CAT_item_list_add(&bag, item_id, 1);
								CAT_room_unstack_prop(deco_state.mod_idx);
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
			deco_state.add_id = -1;
			break;
		}
	}
}

void CAT_render_deco(int cycle)
{
	if(cycle == 0)
	{
		if(deco_state.mode == ADD)
		{
			if(deco_state.add_id != -1)
			{
				int tile_sprite = deco_state.valid_add ? tile_hl_add_sprite : tile_hl_rm_sprite;
				for(int y = deco_state.add_rect.min.y; y < deco_state.add_rect.max.y; y++)
				{
					for(int x = deco_state.add_rect.min.x; x < deco_state.add_rect.max.x; x++)
					{
						CAT_ivec2 place = CAT_grid2world((CAT_ivec2) {x, y});
						CAT_draw_queue_add(tile_sprite, 0, 3, place.x, place.y, CAT_DRAW_MODE_DEFAULT);
					}
				}
			}
			else
			{
				CAT_ivec2 place = CAT_grid2world(room.grid_cursor);
				CAT_draw_queue_add(cursor_add_sprite, 0, 3, place.x, place.y, CAT_DRAW_MODE_DEFAULT);
			}
		}
		else
		{
			int tile_hl = deco_state.mode == FLIP ? tile_hl_flip_sprite : tile_hl_rm_sprite;
			int tile_mark = deco_state.mode == FLIP ? tile_mark_flip_sprite : tile_mark_rm_sprite;
			int cursor = deco_state.mode == FLIP ? cursor_flip_sprite : cursor_remove_sprite;
			if(deco_state.mod_idx != -1)
			{
				for(int y = deco_state.mod_rect.min.y; y < deco_state.mod_rect.max.y; y++)
				{
					for(int x = deco_state.mod_rect.min.x; x < deco_state.mod_rect.max.x; x++)
					{
						CAT_ivec2 place = CAT_grid2world((CAT_ivec2) {x, y});
						CAT_draw_queue_add(tile_hl, 0, 3, place.x, place.y, CAT_DRAW_MODE_DEFAULT);
					}
				}
				CAT_ivec2 place = CAT_grid2world(room.grid_cursor);
				CAT_draw_queue_add(tile_mark, 0, 3, place.x, place.y, CAT_DRAW_MODE_DEFAULT);
			}
			else
			{
				CAT_ivec2 place = CAT_grid2world(room.grid_cursor);
				CAT_draw_queue_add(cursor, 0, 3, place.x, place.y, CAT_DRAW_MODE_DEFAULT);
			}
		}
	}
}