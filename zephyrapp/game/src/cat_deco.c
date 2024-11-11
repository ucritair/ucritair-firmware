#include "cat_deco.h"

#include "cat_item.h"
#include "cat_room.h"
#include "cat_machine.h"
#include "cat_bag.h"
#include "cat_input.h"
#include "cat_pet.h"
#include "cat_sprite.h"
#include <stdio.h>

CAT_deco_state deco_state =
{
	.mode = ADD,

	.add_id = -1,
	.add_rect = (CAT_rect){(CAT_ivec2){0, 0}, (CAT_ivec2){0, 0}},
	.valid_add = false,

	.mod_idx = -1,
	.mod_rect = (CAT_rect){(CAT_ivec2){0, 0}, (CAT_ivec2){0, 0}}
};

void CAT_deco_target(CAT_ivec2 place)
{
	for(int i = 0; i < room.prop_count; i++)
	{
		int item_id = room.prop_ids[i];
		CAT_item* prop = &item_table.data[item_id];
		CAT_ivec2 shape = prop->data.prop_data.shape;
		CAT_rect bounds = CAT_rect_place(room.prop_places[i], shape);
		bounds.max.x -= 1;
		bounds.max.y -= 1;

		if(CAT_rect_pt(place, bounds))
		{
			deco_state.mod_idx = i;
			deco_state.mod_rect = bounds;
			return;
		}
	}

	deco_state.mod_idx = -1;
	deco_state.mod_rect = (CAT_rect) {{0, 0}, {0, 0}};
}

void CAT_MS_deco(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_AM_transition(&pet_asm, &AS_idle);
			deco_state.mode = ADD;
			deco_state.mod_idx = -1;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_room_move_cursor();

			if(CAT_input_pressed(CAT_BUTTON_SELECT))
			{
				deco_state.mode += 1;
				if(deco_state.mode > REMOVE)
					deco_state.mode = ADD;
			}

			switch(deco_state.mode)
			{
				case ADD:
				{
					if(deco_state.add_id != -1)
					{
						CAT_item* prop = &item_table.data[deco_state.add_id];
						CAT_ivec2 shape = prop->data.prop_data.shape;
						CAT_rect bounds = CAT_rect_place(room.cursor, shape);

						deco_state.add_rect = bounds;
						deco_state.valid_add = CAT_room_fits(bounds);

						if(deco_state.valid_add)
						{
							if(CAT_input_pressed(CAT_BUTTON_A))
							{
								CAT_room_add_prop(deco_state.add_id, room.cursor);
								CAT_bag_remove(deco_state.add_id);
								deco_state.add_id = -1;
							}
						}
					}
					else
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							bag_anchor = CAT_MS_deco;
							CAT_machine_transition(&machine, CAT_MS_bag);
						}
					}
					break;
				}
				case FLIP:
				{
					CAT_deco_target(room.cursor);
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
					CAT_deco_target(room.cursor);
					if(deco_state.mod_idx != -1)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
						{
							int item_id = room.prop_ids[deco_state.mod_idx];
							CAT_bag_add(item_id);
							CAT_room_remove_prop(deco_state.mod_idx);
							deco_state.mod_idx = -1;
						}
					}
					break;
				}
			}
		
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_room);
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
						CAT_draw_queue_add(tile_sprite, 0, 3, x * 16, y * 16, CAT_DRAW_MODE_DEFAULT);
					}
				}
			}
			else
			{
				CAT_draw_queue_add(cursor_add_sprite, 0, 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}
		}
		else
		{
			int tile_hl = deco_state.mode == FLIP ? tile_hl_flip_sprite : tile_hl_rm_sprite;
			int tile_mark = deco_state.mode == FLIP ? tile_mark_flip_sprite : tile_mark_rm_sprite;
			int cursor = deco_state.mode == FLIP ? cursor_flip_sprite : cursor_remove_sprite;
			if(deco_state.mod_idx != -1)
			{
				for(int y = deco_state.mod_rect.min.y; y <= deco_state.mod_rect.max.y; y++)
				{
					for(int x = deco_state.mod_rect.min.x; x <= deco_state.mod_rect.max.x; x++)
					{
						CAT_draw_queue_add(tile_hl, 0, 3, x * 16, y * 16, CAT_DRAW_MODE_DEFAULT);
					}
				}
				CAT_draw_queue_add(tile_mark, 0, 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}
			else
			{
				CAT_draw_queue_add(cursor, 0, 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}
		}
	}
}