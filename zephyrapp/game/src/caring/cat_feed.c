#include "cat_actions.h"

#include "cat_math.h"
#include "cat_input.h"
#include "cat_render.h"
#include "sprite_assets.h"
#include "item_assets.h"
#include "cat_item.h"
#include "cat_bag.h"
#include "cowtools/cat_structures.h"
#include "cat_gui.h"
#include "cowtools/cat_curves.h"
#include "cat_pet.h"

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
static bool food_active_mask[5];
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

float group_diversity()
{
	float veg = 0;
	float starch = 0;
	float meat = 0;
	float dairy = 0;

	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!food_active_mask[i])
			continue;	
		CAT_item* food = food_get(i);
		if(food->data.tool_data.food_group == CAT_FOOD_GROUP_VEG && veg < 2)
			veg += 1;
		else if(food->data.tool_data.food_group == CAT_FOOD_GROUP_STARCH && starch < 1)
			starch += 1;
		else if(food->data.tool_data.food_group == CAT_FOOD_GROUP_MEAT && meat < 1)
			meat += 1;
		else if(food->data.tool_data.food_group == CAT_FOOD_GROUP_DAIRY && dairy < 1)
			dairy += 1;
	}

	return (veg + starch + meat + dairy) / 5.0f;
}

float role_propriety()
{
	float treat = 0;
	float vice = 0;

	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!food_active_mask[i])
			continue;	
		CAT_item* food = food_get(i);
		if(food->data.tool_data.food_role == CAT_FOOD_ROLE_TREAT && treat < 2)
			treat += 1;
		else if(food->data.tool_data.food_role == CAT_FOOD_ROLE_VICE)
			vice += 1;
	}

	treat = treat == 1 ? 0 : treat;
	return -(treat + vice) / 2.0f;
}

// % presence of 1 staple, 1 soup, 1 main, 2 sides
float ichiju_sansai()
{
	float staple = 0;
	float soup = 0;
	float main = 0;
	float sides = 0;

	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!food_active_mask[i])
			continue;	
		CAT_item* food = food_get(i);
		if(food->data.tool_data.food_role == CAT_FOOD_ROLE_SOUP && soup < 1)
			soup += 1;
		else if(food->data.tool_data.food_role == CAT_FOOD_ROLE_SIDE && sides < 2)
			sides += 1;
		else if(food->data.tool_data.food_role == CAT_FOOD_ROLE_MAIN && main < 1)
			main += 1;
		else if(food->data.tool_data.food_role == CAT_FOOD_ROLE_STAPLE && staple < 1)
			staple += 1;
	}

	return (staple + soup + main + sides) / 5.0f;
}

static float group_score = 0.0f;
static float role_score = 0.0f;
static float ichisan_score = 0.0f;
static float aggregate_score = 0.0f;
static int level = 0;

static bool commit = false;

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
					if(CAT_gui_popup_is_open())
						break;
					if(CAT_input_pressed(CAT_BUTTON_A))
						CAT_gui_open_popup("Submit this meal?\nUsed food items\nwill be consumed!", &commit);
					if(commit)
					{
						commit = false;
						pet.vigour += level;
						for(int i = 0; i < food_idxs_l.length; i++)
							CAT_item_list_remove(&bag, food_idxs[i], 1);
						CAT_machine_back();
					}

					if(CAT_input_pressed(CAT_BUTTON_SELECT))
					{
						mode = SELECT;
						break;
					}

					if(!CAT_input_touching())
					{
						if(CAT_input_touch_up())
						{
							group_score = group_diversity();
							role_score = role_propriety();
							ichisan_score = ichiju_sansai();
							aggregate_score = (group_score * 3 + role_score * 2 + ichisan_score) / 5.0f;
							if(aggregate_score >= 0.5f)
								aggregate_score *= 1.15f;
							level =
							aggregate_score >= 0.75f ? 4 :
							aggregate_score >= 0.5f ? 3 :
							aggregate_score >= 0.25f ? 2 :
							1;
						}
						touched = -1;
					}
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

					for(int i = 0; i < food_idxs_l.length; i++)
						food_active_mask[i] = CAT_rect_contains(table_rect, food_rects[i]);
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
				CAT_push_draw_scale(3);
				CAT_draw_sprite(food->sprite, 0, col * w, row * h);

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

			for(int i = food_idxs_l.length-1; i >= 0; i--)
			{
				CAT_item* food = food_get(i);
				CAT_rect rect = food_rects[i];
				int x = rect.min.x;
				int y = rect.min.y;
				CAT_push_draw_scale(3);
				CAT_draw_sprite(food->sprite, 0, x, y);
			}

			CAT_push_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
			CAT_push_draw_colour
			(
				level == 4 ? CAT_PURPLE :
				level == 3 ? CAT_GREEN :
				level == 2 ? CAT_YELLOW :
				CAT_RED
			);
			CAT_draw_sprite(&gizmo_face_96x96_sprite, 0, 120, 290);

			CAT_gui_printf(CAT_WHITE, "group diversity: %0.2f", group_score);
			CAT_gui_printf(CAT_WHITE, "role propriety: %0.2f", role_score);
			CAT_gui_printf(CAT_WHITE, "ichiju sansai: %0.2f", ichisan_score);
			CAT_gui_printf(CAT_WHITE, "aggregate: %0.2f", aggregate_score);
			break;
		}
	}
}