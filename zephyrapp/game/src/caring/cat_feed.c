#include "cat_actions.h"

#include "cat_math.h"
#include "cat_input.h"
#include "cat_render.h"
#include "sprite_assets.h"
#include "item_assets.h"
#include "cat_item.h"
#include "cat_bag.h"
#include "cowtools/cat_structures.h"

static enum
{
	SELECT,
	ARRANGE
} mode = ARRANGE;

static CAT_rect menu_rect =
{
	.min = {0, 0},
	.max = {240, 80}
};

static CAT_rect spawn_rects[5] =
{
	{{32, 24}, {32+48, 24+48}},
	{{64, 24}, {64+48, 24+48}},
	{{96, 24}, {96+48, 24+48}},
	{{128, 24}, {128+48, 24+48}},
	{{160, 24}, {160+48, 24+48}}
};

static CAT_rect table_rect =
{
	.min = {0, 80},
	.max = {240, 240}
};

static CAT_item_list food_pool;
static int idx_pool[CAT_ITEM_LIST_MAX_LENGTH];
static int food_idxs[5];
static CAT_rect food_rects[5];
static CAT_int_list food_idxs_l;

bool food_filter(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return false;
	return
	item->type == CAT_ITEM_TYPE_TOOL &&
	item->data.tool_data.type == CAT_TOOL_TYPE_FOOD;
}

void food_swap(int i, int j)
{
	int temp_id = food_idxs[i];
	food_idxs[i] = food_idxs[j];
	food_idxs[j] = temp_id;
	CAT_rect temp_rect = food_rects[i];
	food_rects[i] = food_rects[j];
	food_rects[j] = temp_rect;
}

CAT_item* food_get(int i)
{
	return CAT_item_get(food_pool.item_ids[food_idxs[i]]);
}

void food_delete(int i)
{
	CAT_ilist_delete(&food_idxs_l, i);
	for(int j = i; j < 5; j++)
		food_rects[j] = food_rects[j+1];
}

CAT_rect rect_overlap(CAT_rect a, CAT_rect b)
{
	return (CAT_rect)
	{
		{ max(a.min.x, b.min.x), max(a.min.y, b.min.y) },
		{ min(a.max.x, b.max.x), min(a.max.y, b.max.y) }
	};
}

float spawn_rect_fill(int rect_idx)
{
	CAT_rect spawn_rect = spawn_rects[rect_idx];
	int spawn_w = spawn_rect.max.x - spawn_rect.min.x;
	int spawn_h = spawn_rect.max.y - spawn_rect.min.y;
	int spawn_a = spawn_w * spawn_h;

	int overlap_a = 0;
	for(int i = 0; i < food_idxs_l.length; i++)
	{
		CAT_rect food_rect = food_rects[i];
		CAT_rect overlap = rect_overlap(spawn_rect, food_rect);
		int w = overlap.max.x - overlap.min.x;
		int h = overlap.max.y - overlap.min.y;
		w = w > 0 ? w : 0;
		h = h > 0 ? h : 0;
		overlap_a += w * h;
	}

	return (float) overlap_a / (float) spawn_a;
}

void food_spawn(int idx)
{
	int free_rect_idx = -1;
	float min_fill = 2.0f;
	for(int i = 0; i < 5; i++)
	{
		float fill = spawn_rect_fill(i);
		if(fill < min_fill)
		{
			free_rect_idx = i;
			min_fill = fill;
		}
	}

	CAT_rect spawn_rect = free_rect_idx != -1 ?
	spawn_rects[free_rect_idx] : (CAT_rect) {{96, 16}, {144, 64}};

	CAT_ilist_push(&food_idxs_l, idx);
	food_rects[food_idxs_l.length-1] = spawn_rect;
}

static int touched = -1;
static int dx, dy;

void CAT_MS_feed(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_feed);

			CAT_item_list_init(&food_pool);
			CAT_item_list_filter(&bag, &food_pool, food_filter);

			for(int i = 0; i < food_pool.length; i++)
				idx_pool[i] = i;
			CAT_int_list idx_pool_l;
			CAT_ilist(&idx_pool_l, idx_pool, food_pool.length);
			CAT_ilist_shuffle(&idx_pool_l);
			
			CAT_ilist(&food_idxs_l, food_idxs, 5);
			for(int i = 0; i < 5; i++)
			{
				food_spawn(idx_pool[i]);
			}
		break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();
			
			switch(mode)
			{
				case SELECT:
				{
					if(CAT_input_pressed(CAT_BUTTON_SELECT))
					{
						mode = ARRANGE;
						break;
					}

					int row = 0;
					int col = 0;
					for(int i = 0; i < food_pool.length; i++)
					{
						CAT_item* food = CAT_item_get(food_pool.item_ids[i]);
						int w = food->sprite->width * 3;
						int h = food->sprite->height * 3;

						if(CAT_input_touch_down() && CAT_input_touch_rect(col * w, row * h, w, h))
						{	
							int food_idxs_idx = CAT_ilist_find(&food_idxs_l, i);
							if(food_idxs_idx != -1)
								food_delete(food_idxs_idx);
							else if(food_idxs_l.length < 5)
								food_spawn(i);
						}	
						
						col += 1;
						if(col >= CAT_LCD_SCREEN_W / w)
						{
							col = 0;
							row += 1;
						}
					}
					break;
				}
				case ARRANGE:
				{
					if(CAT_input_pressed(CAT_BUTTON_SELECT))
					{
						mode = SELECT;
						break;
					}

					if(!CAT_input_touching())
						touched = -1;
					else if(CAT_input_touch_down())
					{
						for(int i = 0; i < food_idxs_l.length; i++)
						{
							int x = food_rects[i].min.x;
							int y = food_rects[i].min.y;
							int w = food_rects[i].max.x - x;
							int h = food_rects[i].max.y - y;
							if(CAT_input_touch_rect(x, y, w, h))
							{
								food_swap(i, 0);
								dx = x - input.touch.x;
								dy = y - input.touch.y;	
								touched = 0;
								break;
							}
						}
					}
					else
					{
						if(touched != -1)
						{
							int w = food_rects[touched].max.x - food_rects[touched].min.x;
							int h = food_rects[touched].max.y - food_rects[touched].min.y;
							food_rects[touched].min.x = input.touch.x + dx;
							food_rects[touched].min.y = input.touch.y + dy;
							food_rects[touched].max.x = food_rects[touched].min.x + w;
							food_rects[touched].max.y = food_rects[touched].min.y + h;
						}
					}
					break;
				}
			}			
		break;
		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_feed()
{
	draw_flags = CAT_DRAW_FLAG_DEFAULT;

	switch(mode)
	{
		case SELECT:
		{
			CAT_frameberry(CAT_WHITE);
			int row = 0;
			int col = 0;
			for(int i = 0; i < food_pool.length; i++)
			{
				CAT_item* food = CAT_item_get(food_pool.item_ids[i]);
				int w = food->sprite->width * 3;
				int h = food->sprite->height * 3;
				CAT_draw_sprite_scaled(food->sprite, 0, col * w, row * h, 3);

				for(int j = 0; j < food_idxs_l.length; j++)
				{
					if(i == food_idxs[j])
						CAT_strokeberry(col * w, row * h, w, h, CAT_RED);
				}
				
				col += 1;
				if(col >= CAT_LCD_SCREEN_W / w)
				{
					col = 0;
					row += 1;
				}
			}
			break;
		}
		case ARRANGE:
		{
			CAT_frameberry(CAT_BLACK);

			int menu_x = menu_rect.min.x;
			int menu_y = menu_rect.min.y;
			int menu_w = menu_rect.max.x - menu_x;
			int menu_h = menu_rect.max.y - menu_y;
			CAT_strokeberry(menu_x, menu_y, menu_w, menu_h, CAT_RED);
			int table_x = table_rect.min.x;
			int table_y = table_rect.min.y;
			int table_w = table_rect.max.x - table_x;
			int table_h = table_rect.max.y - table_y;
			CAT_strokeberry(table_x, table_y, table_w, table_h, CAT_BLUE);

			draw_flags = CAT_DRAW_FLAG_DEFAULT;
			for(int i = food_idxs_l.length-1; i >= 0; i--)
			{
				CAT_item* food = food_get(i);
				CAT_rect rect = food_rects[i];
				int x = rect.min.x;
				int y = rect.min.y;
				CAT_draw_sprite_scaled(food->sprite, 0, x, y, 3);
			}
			break;
		}
	}
	
}