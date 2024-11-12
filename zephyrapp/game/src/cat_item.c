#include "cat_item.h"
#include <stdio.h>
#include "cat_sprite.h"

//////////////////////////////////////////////////////////////////////////
// ITEM TABLE

CAT_item_table item_table;

void CAT_item_table_init()
{
	item_table.length = 0;
}

int CAT_item_init(CAT_item_type type, const char* name, int sprite_id, int price)
{
	int item_id = item_table.length;
	item_table.length += 1;

	CAT_item* item = &item_table.data[item_id];
	item->type = type;
	item->name = name;
	item->sprite_id = sprite_id;
	item->price = price;

	return item_id;
}

CAT_item* CAT_item_get(int item_id)
{
	return &item_table.data[item_id];
}

void CAT_tool_init(int item_id, int cursor_sprite_id, int dv, int df, int ds, bool consumable)
{
	CAT_item* item = CAT_item_get(item_id);
	item->data.tool_data.cursor_sprite_id = cursor_sprite_id;
	item->data.tool_data.dv = dv;
	item->data.tool_data.df = df;
	item->data.tool_data.ds = ds;
	item->data.tool_data.consumable = consumable;
}

void CAT_prop_init(int item_id, int width, int height, bool animate)
{
	CAT_item* item = CAT_item_get(item_id);
	item->data.prop_data.shape = (CAT_ivec2) {width, height};
	item->data.prop_data.animate = animate;
}

void CAT_gear_init(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	item->data.gear_data.equipped = false;
}

void CAT_gear_toggle(int item_id, bool equipped)
{
	CAT_item* item = CAT_item_get(item_id);
	item->data.gear_data.equipped = equipped;
}

bool CAT_gear_status(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	return item->data.gear_data.equipped;
}


//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

// TOOLS
int padkaprow_item;
int sausage_item;
int coffee_item;
int salad_item;
int pill_vig_item;
int pill_foc_item;
int pill_spi_item;
int cigarette_item;

int book_a_item;
int book_b_item;
int book_c_item;
int book_d_item;
int book_e_item;
int book_f_item;

int toy_duck_item;
int toy_baseball_item;
int toy_basketball_item;
int toy_golf_item;
int toy_puzzle_item;

// KEYS AND GEAR
int mask_item;

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

void CAT_item_mass_define()
{
	// TOOLS
	pill_vig_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Vigour Pill", pill_vig_sprite, 2);
	CAT_tool_init(pill_vig_item, pill_vig_sprite, 1, 0, 0, true);

	pill_foc_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Focus Pill", pill_foc_sprite, 2);
	CAT_tool_init(pill_foc_item, pill_foc_sprite, 0, 1, 0, true);

	pill_spi_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Spirit Pill", pill_spi_sprite, 2);
	CAT_tool_init(pill_spi_item, pill_spi_sprite, 0, 0, 1, true);

	padkaprow_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Pad Ka Prow", padkaprow_sprite, 10);
	CAT_tool_init(padkaprow_item, padkaprow_sprite, 5, 0, 0, true);

	sausage_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Sausage", sausage_sprite, 5);
	CAT_tool_init(sausage_item, sausage_sprite, 3, 0, 0, true);

	coffee_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Coffee", coffee_sprite, 2);
	CAT_tool_init(coffee_item, coffee_sprite, 1, 1, 0, true);

	salad_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Salad", salad_item, 5);
	CAT_tool_init(salad_item, salad_sprite, 3, 0, 0, true);

	cigarette_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Cigarettes", cigarette_sprite, 10);
	CAT_tool_init(cigarette_item, cigarette_sprite, -1, 3, 1, true);


	book_f_item = CAT_item_init(CAT_ITEM_TYPE_BOOK, "The Disposessed", book_study_sprite, 5);
	CAT_tool_init(book_f_item, book_static_sprite, 0, 1, 0, false);

	book_a_item = CAT_item_init(CAT_ITEM_TYPE_BOOK, "Rendezvous with Rama", book_study_sprite, 20);
	CAT_tool_init(book_a_item, book_static_sprite, 0, 3, 0, false);

	book_b_item = CAT_item_init(CAT_ITEM_TYPE_BOOK, "The Forever War", book_study_sprite, 20);
	CAT_tool_init(book_b_item, book_static_sprite, 0, 3, 0, false);

	book_c_item = CAT_item_init(CAT_ITEM_TYPE_BOOK, "Ringworld", book_study_sprite, 20);
	CAT_tool_init(book_c_item, book_static_sprite, 0, 3, 0, false);

	book_d_item = CAT_item_init(CAT_ITEM_TYPE_BOOK, "The Machine Stops", book_study_sprite, 20);
	CAT_tool_init(book_d_item, book_static_sprite, 0, 3, 0, false);

	book_e_item = CAT_item_init(CAT_ITEM_TYPE_BOOK, "Metal Fever", book_study_sprite, 20);
	CAT_tool_init(book_e_item, book_static_sprite, 0, 3, 0, false);


	toy_golf_item = CAT_item_init(CAT_ITEM_TYPE_TOY, "Golf Ball", toy_golf_sprite, 5);
	CAT_tool_init(toy_golf_item, toy_golf_sprite, 0, 0, 1, false);

	toy_baseball_item = CAT_item_init(CAT_ITEM_TYPE_TOY, "Baseball", toy_baseball_sprite, 20);
	CAT_tool_init(toy_baseball_item, toy_baseball_sprite, 0, 0, 3, false);

	toy_basketball_item = CAT_item_init(CAT_ITEM_TYPE_TOY, "Basketball", toy_basketball_sprite, 20);
	CAT_tool_init(toy_basketball_item, toy_basketball_sprite, 0, 0, 3, false);

	toy_puzzle_item = CAT_item_init(CAT_ITEM_TYPE_TOY, "Puzzle", toy_puzzle_sprite, 20);
	CAT_tool_init(toy_puzzle_item, toy_puzzle_sprite, 0, 0, 3, false);

	toy_duck_item = CAT_item_init(CAT_ITEM_TYPE_TOY, "Ducky", toy_duck_sprite, 30);
	CAT_tool_init(toy_duck_item, toy_duck_sprite, 0, 0, 5, false);
	

	// KEYS AND GEAR
	mask_item = CAT_item_init(CAT_ITEM_TYPE_GEAR, "Mask", icon_mask_sprite, 10);
	CAT_gear_init(mask_item);


	// PROPS
	chair_wood_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Wood Chair", chair_wood_sprite, 5);
	CAT_prop_init(chair_wood_item, 2, 2, false);

	table_sm_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Small Table", table_sm_sprite, 10);
	CAT_prop_init(table_sm_item, 2, 2, true);

	table_lg_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Large Table", table_lg_sprite, 20);
	CAT_prop_init(table_lg_item, 4, 2, true);

	lantern_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Lantern", lantern_sprite, 20);
	CAT_prop_init(lantern_item, 1, 1, true);

	fan_a_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Classic Fan", fan_a_sprite, 20);
	CAT_prop_init(fan_a_item, 2, 1, true);

	fan_b_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Modern Fan", fan_b_sprite, 30);
	CAT_prop_init(fan_b_item, 2, 1, true);

	coffeemaker_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Coffee Machine", coffeemaker_sprite, 40);
	CAT_prop_init(coffeemaker_item, 2, 1, true);
	

	uv_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "UV Lamp", uv_sprite, 50);
	CAT_prop_init(uv_item, 1, 1, true);

	purifier_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Air Purifier", purifier_sprite, 50);
	CAT_prop_init(purifier_item, 2, 1, true);

	gpu_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Ethereum Farm", gpu_sprite, 100);
	CAT_prop_init(gpu_item, 3, 1, true);

	
	stool_wood_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Wood Stool", stool_wood_sprite, 10);
	CAT_prop_init(stool_wood_item, 2, 1, true);

	stool_stone_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Stone Stool", stool_stone_sprite, 10);
	CAT_prop_init(stool_stone_item, 2, 1, true);

	stool_gold_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Gold Stool", stool_gold_sprite, 20);
	CAT_prop_init(stool_gold_item, 2, 1, true);

	bowl_stone_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Stone Bowl", bowl_stone_sprite, 10);
	CAT_prop_init(bowl_stone_item, 1, 1, true);

	bowl_gold_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Gold Bowl", bowl_gold_sprite, 20);
	CAT_prop_init(bowl_gold_item, 1, 1, true);


	succulent_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Succulent", succulent_sprite, 5);
	CAT_prop_init(succulent_item, 1, 1, false);

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

	effigy_blue_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Blue Effigy", effigy_blue_sprite, 40);
	CAT_prop_init(effigy_blue_item, 3, 1, true);
	
	effigy_purple_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Purple Effigy", effigy_purple_sprite, 40);
	CAT_prop_init(effigy_purple_item, 3, 1, true);
	
	effigy_sea_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Sea Effigy", effigy_sea_sprite, 60);
	CAT_prop_init(effigy_sea_item, 3, 1, true);

	pixel_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Pixel", pixel_sprite, 30);
	CAT_prop_init(pixel_item, 1, 1, true);
}
