#include "cat_item.h"

#include <stdio.h>
#include "cat_sprite.h"
#include <string.h>
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////
// ITEM TABLE

CAT_item_table item_table;

void CAT_item_table_init()
{
	item_table.length = 0;
}

int CAT_item_init(CAT_item_type type, const char* name, int sprite_id, int price)
{
	if(item_table.length >= CAT_ITEM_TABLE_MAX_LENGTH)
	{
		CAT_printf("[WARNING] attempted add to full item table\n");
		return -1;
	}

	int item_id = item_table.length;
	item_table.length += 1;

	CAT_item* item = &item_table.data[item_id];
	item->type = type;
	item->name = name;
	item->sprite_id = sprite_id;
	item->price = price;
	item->text = "";
	item->icon_id = icon_item_key_sprite;

	return item_id;
}

CAT_item* CAT_item_get(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
	{
		return NULL;
	}
		
	return &item_table.data[item_id];
}

void CAT_tool_init(int item_id, CAT_tool_type type, int cursor_id, int dv, int df, int ds)
{
	if(item_id < 0 || item_id >= item_table.length)
	{
		CAT_printf("[ERROR] reference to invalid item id: %d\n", item_id);
		return;
	}

	CAT_item* item = CAT_item_get(item_id);
	item->icon_id = icon_item_food_sprite + type;
	item->data.tool_data.type = type;
	item->data.tool_data.cursor_id = cursor_id;
	item->data.tool_data.dv = dv;
	item->data.tool_data.df = df;
	item->data.tool_data.ds = ds;
}

void CAT_prop_init(int item_id, int width, int height, bool animate)
{
	if(item_id < 0 || item_id >= item_table.length)
	{
		CAT_printf("[ERROR] reference to invalid item id: %d\n", item_id);
		return;
	}

	CAT_item* item = CAT_item_get(item_id);
	item->icon_id = icon_item_prop_sprite;
	item->data.prop_data.type = CAT_PROP_TYPE_DEFAULT;
	item->data.prop_data.shape = (CAT_ivec2) {width, height};
	item->data.prop_data.animate = animate;
}


//////////////////////////////////////////////////////////////////////////
// ITEM LIST

void CAT_item_list_init(CAT_item_list* item_list)
{
	item_list->length = 0;
}

int CAT_item_list_find(CAT_item_list* item_list, int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
	{
		CAT_printf("[ERROR] reference to invalid item id: %d\n", item_id);
		return -1;
	}

	for(int i = 0; i < item_list->length; i++)
	{
		if(item_list->item_ids[i] == item_id)
		{
			return i;
		}
	}
	return -1;
}

void CAT_item_list_add(CAT_item_list* item_list, int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
	{
		CAT_printf("[ERROR] reference to invalid item id: %d\n", item_id);
		return;
	}
	if(item_list->length >= CAT_ITEM_LIST_MAX_LENGTH)
	{
		CAT_printf("[WARNING] attempted add to full item list\n");
		return;
	}
		
	int idx = CAT_item_list_find(item_list, item_id);
	if(idx >= 0)
	{
		item_list->counts[idx] += 1;
	}
	else
	{
		const char* a = item_table.data[item_id].name;
		int insert_idx = 0;
		while(insert_idx < item_list->length)
		{
			const char* b = item_table.data[item_list->item_ids[insert_idx]].name;
			if(strcmp(a, b) < 0)
				break;
			insert_idx += 1;
		}
		for(int i = item_list->length; i > insert_idx; i--)
		{
			item_list->item_ids[i] = item_list->item_ids[i-1];
			item_list->counts[i] = item_list->counts[i-1];
		}
		item_list->item_ids[insert_idx] = item_id;
		item_list->counts[insert_idx] = 1;
		item_list->length += 1;
	}
}

void CAT_item_list_remove(CAT_item_list* item_list, int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
	{
		CAT_printf("[ERROR] reference to invalid item id: %d\n", item_id);
		return;
	}

	int idx = CAT_item_list_find(item_list, item_id);
	if(idx >= 0)
	{
		item_list->counts[idx] -= 1;
		if(item_list->counts[idx] <= 0)
		{
			for(int i = idx; i < item_list->length-1; i++)
			{
				item_list->item_ids[i] = item_list->item_ids[i+1];
				item_list->counts[i] = item_list->counts[i+1];
			}
			item_list->length -= 1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

// KEYS
int mask_item;

// FOOS
int padkaprow_item;
int sausage_item;
int coffee_item;
int salad_item;
int pill_vig_item;
int pill_foc_item;
int pill_spi_item;
int cigarette_item;

// BOOKS
int book_a_item;
int book_b_item;
int book_c_item;
int book_d_item;
int book_e_item;
int book_f_item;

// TOYS
int toy_duck_item;
int toy_baseball_item;
int toy_basketball_item;
int toy_golf_item;
int toy_puzzle_item;

// PROPS
int gpu_item;
int uv_item;
int purifier_item;

int coffeemaker_item;
int fan_a_item;
int fan_b_item;
int lantern_item;

int table_lg_item;
int table_sm_item;
int chair_wood_item;
int stool_wood_item;
int stool_stone_item;
int stool_gold_item;

int bowl_stone_item;
int bowl_gold_item;
int vase_stone_item;
int vase_gold_item;

int succulent_item;
int bush_plain_item;
int bush_daisy_item;
int bush_lilac_item;
int plant_green_item;
int plant_maroon_item;
int plant_purple_item;
int plant_yellow_item;
int flower_vig_item;
int flower_foc_item;
int flower_spi_item;

int crystal_blue_lg_item;
int crystal_green_lg_item;
int crystal_purple_lg_item;

int effigy_blue_item;
int effigy_purple_item;
int effigy_sea_item;

int pixel_item;
int padkaprop_item;

void CAT_item_mass_define()
{
	// KEYS
	mask_item = CAT_item_init(CAT_ITEM_TYPE_KEY, "Mask", icon_mask_sprite, 10);

	// FOOD
	pill_vig_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Vigour Pill", pill_vig_sprite, 2);
	CAT_tool_init(pill_vig_item, CAT_TOOL_TYPE_FOOD, pill_vig_sprite, 1, 0, 0);

	pill_foc_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Focus Pill", pill_foc_sprite, 2);
	CAT_tool_init(pill_foc_item, CAT_TOOL_TYPE_FOOD, pill_foc_sprite, 0, 1, 0);

	pill_spi_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Spirit Pill", pill_spi_sprite, 2);
	CAT_tool_init(pill_spi_item, CAT_TOOL_TYPE_FOOD, pill_spi_sprite, 0, 0, 1);

	padkaprow_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Pad Ka Prow", padkaprow_sprite, 10);
	CAT_tool_init(padkaprow_item, CAT_TOOL_TYPE_FOOD, padkaprow_sprite, 5, 0, 0);

	sausage_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Sausage", sausage_sprite, 5);
	CAT_tool_init(sausage_item, CAT_TOOL_TYPE_FOOD, sausage_sprite, 3, 0, 0);

	coffee_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Coffee", coffee_sprite, 2);
	CAT_tool_init(coffee_item, CAT_TOOL_TYPE_FOOD, coffee_sprite, 1, 1, 0);

	salad_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Salad", salad_sprite, 5);
	CAT_tool_init(salad_item, CAT_TOOL_TYPE_FOOD, salad_sprite, 3, 0, 0);

	cigarette_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Cigarettes", cigarette_sprite, 10);
	CAT_tool_init(cigarette_item, CAT_TOOL_TYPE_FOOD, cigarette_sprite, -1, 3, 1);

	// BOOKS
	book_f_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "The Disposessed", book_study_sprite, 20);
	CAT_tool_init(book_f_item, CAT_TOOL_TYPE_BOOK, book_static_sprite, 0, 3, 0);

	book_a_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "We Are Legion", book_study_sprite, 20);
	CAT_tool_init(book_a_item, CAT_TOOL_TYPE_BOOK, book_static_sprite, 0, 3, 0);

	book_b_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "The Forever War", book_study_sprite, 20);
	CAT_tool_init(book_b_item, CAT_TOOL_TYPE_BOOK, book_static_sprite, 0, 3, 0);

	book_c_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Ringworld", book_study_sprite, 20);
	CAT_tool_init(book_c_item, CAT_TOOL_TYPE_BOOK, book_static_sprite, 0, 3, 0);

	book_d_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "The Machine Stops", book_study_sprite, 20);
	CAT_tool_init(book_d_item, CAT_TOOL_TYPE_BOOK, book_static_sprite, 0, 3, 0);

	book_e_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Metal Fever", book_study_sprite, 20);
	CAT_tool_init(book_e_item, CAT_TOOL_TYPE_BOOK, book_static_sprite, 0, 3, 0);

	// TOYS
	toy_golf_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Golf Ball", toy_golf_sprite, 5);
	CAT_tool_init(toy_golf_item, CAT_TOOL_TYPE_TOY, toy_golf_sprite, 0, 0, 1);

	toy_baseball_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Baseball", toy_baseball_sprite, 20);
	CAT_tool_init(toy_baseball_item, CAT_TOOL_TYPE_TOY, toy_baseball_sprite, 0, 0, 3);

	toy_basketball_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Basketball", toy_basketball_sprite, 20);
	CAT_tool_init(toy_basketball_item, CAT_TOOL_TYPE_TOY, toy_basketball_sprite, 0, 0, 3);

	toy_puzzle_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Puzzle", toy_puzzle_sprite, 20);
	CAT_tool_init(toy_puzzle_item, CAT_TOOL_TYPE_TOY, toy_puzzle_sprite, 0, 0, 3);

	toy_duck_item = CAT_item_init(CAT_ITEM_TYPE_TOOL, "Ducky", toy_duck_sprite, 30);
	CAT_tool_init(toy_duck_item, CAT_TOOL_TYPE_TOY, toy_duck_sprite, 0, 0, 5);

	// PROPS
	chair_wood_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Wood Chair", chair_wood_sprite, 5);
	CAT_prop_init(chair_wood_item, 2, 2, false);

	table_sm_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Small Table", table_sm_sprite, 10);
	CAT_prop_init(table_sm_item, 2, 2, true);
	item_table.data[table_sm_item].data.prop_data.type = CAT_PROP_TYPE_BOTTOM;

	table_lg_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Large Table", table_lg_sprite, 20);
	CAT_prop_init(table_lg_item, 4, 2, true);
	item_table.data[table_lg_item].data.prop_data.type = CAT_PROP_TYPE_BOTTOM;

	lantern_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Lantern", lantern_sprite, 20);
	CAT_prop_init(lantern_item, 1, 1, true);
	item_table.data[lantern_item].data.prop_data.type = CAT_PROP_TYPE_TOP;

	fan_a_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Classic Fan", fan_a_sprite, 20);
	CAT_prop_init(fan_a_item, 2, 1, true);
	item_table.data[fan_a_item].data.prop_data.type = CAT_PROP_TYPE_TOP;

	fan_b_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Modern Fan", fan_b_sprite, 30);
	CAT_prop_init(fan_b_item, 2, 1, true);

	coffeemaker_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Coffee Machine", coffeemaker_sprite, 40);
	CAT_prop_init(coffeemaker_item, 2, 1, true);
	item_table.data[coffeemaker_item].data.prop_data.type = CAT_PROP_TYPE_TOP;
	

	uv_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "UV Lamp", uv_sprite, 50);
	CAT_prop_init(uv_item, 1, 1, true);

	purifier_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Air Purifier", purifier_sprite, 50);
	CAT_prop_init(purifier_item, 2, 1, true);

	gpu_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Ethereum Farm", gpu_sprite, 100);
	CAT_prop_init(gpu_item, 3, 1, true);

	
	stool_wood_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Wood Stool", stool_wood_sprite, 10);
	CAT_prop_init(stool_wood_item, 2, 1, true);
	item_table.data[stool_wood_item].data.prop_data.type = CAT_PROP_TYPE_BOTTOM;

	stool_stone_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Stone Stool", stool_stone_sprite, 10);
	CAT_prop_init(stool_stone_item, 2, 1, true);
	item_table.data[stool_stone_item].data.prop_data.type = CAT_PROP_TYPE_BOTTOM;

	stool_gold_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Gold Stool", stool_gold_sprite, 20);
	CAT_prop_init(stool_gold_item, 2, 1, true);
	item_table.data[stool_gold_item].data.prop_data.type = CAT_PROP_TYPE_BOTTOM;

	bowl_stone_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Stone Bowl", bowl_stone_sprite, 10);
	CAT_prop_init(bowl_stone_item, 1, 1, true);
	item_table.data[bowl_stone_item].data.prop_data.type = CAT_PROP_TYPE_TOP;

	bowl_gold_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Gold Bowl", bowl_gold_sprite, 20);
	CAT_prop_init(bowl_gold_item, 1, 1, true);
	item_table.data[bowl_gold_item].data.prop_data.type = CAT_PROP_TYPE_TOP;


	succulent_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Succulent", succulent_sprite, 5);
	CAT_prop_init(succulent_item, 1, 1, false);
	item_table.data[succulent_item].data.prop_data.type = CAT_PROP_TYPE_TOP;

	bush_plain_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Bush", bush_plain_sprite, 10);
	CAT_prop_init(bush_plain_item, 2, 1, true);

	bush_daisy_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Daisy Bush", bush_daisy_sprite, 30);
	CAT_prop_init(bush_daisy_item, 2, 1, true);

	bush_lilac_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Lilac Bush", bush_lilac_sprite, 30);
	CAT_prop_init(bush_lilac_item, 2, 1, true);

	plant_green_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Green Plant", plant_green_sprite, 20);
	CAT_prop_init(plant_green_item, 2, 1, true);

	plant_maroon_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Maroon Plant", plant_maroon_sprite, 20);
	CAT_prop_init(plant_maroon_item, 2, 1, true);

	plant_purple_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Purple Plant", plant_purple_sprite, 20);
	CAT_prop_init(plant_purple_item, 2, 1, true);

	plant_yellow_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Yellow Plant", plant_yellow_sprite, 20);
	CAT_prop_init(plant_yellow_item, 2, 1, true);

	flower_vig_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Healthy Flower", flower_vig_sprite, 40);
	CAT_prop_init(flower_vig_item, 2, 1, false);

	flower_foc_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Wise Flower", flower_foc_sprite, 40);
	CAT_prop_init(flower_foc_item, 2, 1, false);

	flower_spi_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Happy Flower", flower_spi_sprite, 40);
	CAT_prop_init(flower_spi_item, 2, 1, false);


	crystal_blue_lg_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Blue Crystal", crystal_blue_lg_sprite, 30);
	CAT_prop_init(crystal_blue_lg_item, 2, 1, true);

	crystal_green_lg_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Green Crystal", crystal_green_lg_sprite, 30);
	CAT_prop_init(crystal_green_lg_item, 2, 1, true);

	crystal_purple_lg_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Purple Crystal", crystal_purple_lg_sprite, 30);
	CAT_prop_init(crystal_purple_lg_item, 2, 1, true);

	effigy_blue_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Blue Hedron", effigy_blue_sprite, 40);
	CAT_prop_init(effigy_blue_item, 3, 1, true);
	
	effigy_purple_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Purple Hedron", effigy_purple_sprite, 40);
	CAT_prop_init(effigy_purple_item, 3, 1, true);
	
	effigy_sea_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Sea Hedron", effigy_sea_sprite, 60);
	CAT_prop_init(effigy_sea_item, 3, 1, true);

	padkaprop_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Pad Ka Prop", padkaprop_sprite, 30);
	CAT_prop_init(padkaprop_item, 2, 1, true);

	pixel_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Pixel", pixel_sprite, 30);
	CAT_prop_init(pixel_item, 1, 1, true);
	item_table.data[pixel_item].data.prop_data.type = CAT_PROP_TYPE_TOP;
}
