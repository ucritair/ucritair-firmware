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

void CAT_food_init(int item_id, float d_v, float d_f, float d_s)
{
	CAT_item* item = CAT_item_get(item_id);
	item->data.food_data.d_v = d_v;
	item->data.food_data.d_f = d_f;
	item->data.food_data.d_s = d_s;
}

void CAT_book_init(int item_id, int icon_sprite_id)
{
	CAT_item* item = CAT_item_get(item_id);
	item->data.book_data.icon_sprite_id = icon_sprite_id;
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

int cigarettes_item;
int sausage_item;
int padkrapow_item;
int coffee_item;
int book_a_item;
int book_b_item;
int book_c_item;
int toy_a_item;
int toy_b_item;
int toy_c_item;
int mask_item;

int solderpaste_item;
int coffeemaker_item;
int fan_item;
int purifier_item;
int uv_item;
int crypto_item;

int chair_wood_item;
int table_sm_item;
int table_lg_item;

int lantern_lit_item;
int bowl_stone_item;
int bowl_gold_item;

int pixel_item;

void CAT_item_mass_define()
{
	// GRIMBA

	cigarettes_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Cigarettes", cigarette_sprite, 1);
	CAT_food_init(cigarettes_item, 0, 0, 1);

	sausage_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Sausage", sausage_sprite, 1);
	CAT_food_init(sausage_item, 1, 0, 0);

	padkrapow_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Pad Ka Prow", padkrapow_sprite, 1);
	CAT_food_init(padkrapow_item, 1, 0, 0);

	coffee_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Coffee", coffee_sprite, 1);
	CAT_food_init(coffee_item, 0, 1, 0);

	book_a_item = CAT_item_init(CAT_ITEM_TYPE_BOOK, "\"Clean Air Handbook\"", book_study_sprite, 1);
	CAT_book_init(book_a_item, book_static_sprite);

	book_b_item = CAT_item_init(CAT_ITEM_TYPE_BOOK, "\"Zero-Knowledge Proofs\"", book_study_sprite, 1);
	CAT_book_init(book_b_item, book_static_sprite);

	book_c_item = CAT_item_init(CAT_ITEM_TYPE_BOOK, "\"The Disposessed\"", book_study_sprite, 1);
	CAT_book_init(book_c_item, book_static_sprite);

	toy_a_item = CAT_item_init(CAT_ITEM_TYPE_TOY, "Ducky", toy_duck_sprite, 1);
	toy_b_item = CAT_item_init(CAT_ITEM_TYPE_TOY, "Baseball", toy_baseball_sprite, 1);
	toy_c_item = CAT_item_init(CAT_ITEM_TYPE_TOY, "Basketball", toy_basketball_sprite, 1);

	mask_item = CAT_item_init(CAT_ITEM_TYPE_GEAR, "Mask", mask_sprite, 1);
	CAT_gear_init(mask_item);


	chair_wood_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Wood Chair", chair_wood_sprite, 1);
	CAT_prop_init(chair_wood_item, 2, 2, false);

	table_sm_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Small Table", table_sm_sprite, 1);
	CAT_prop_init(table_sm_item, 2, 2, false);

	table_lg_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Large Table", table_lg_sprite, 1);
	CAT_prop_init(table_lg_item, 4, 2, false);


	coffeemaker_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Coffee Machine", coffeemaker_sprite, 1);
	CAT_prop_init(coffeemaker_item, 2, 2, true);

	fan_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Fan", fan_sprite, 1);
	CAT_prop_init(fan_item, 2, 1, true);

	purifier_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Air Purifier", purifier_sprite, 1);
	CAT_prop_init(purifier_item, 2, 1, true);

	uv_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "UV Lamp", uv_lamp_sprite, 1);
	CAT_prop_init(uv_item, 1, 1, true);

	crypto_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Ethereum Farm", solderpaste_sprite, 1);
	CAT_prop_init(crypto_item, 3, 1, true);


	lantern_lit_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Lantern (Lit)", lantern_lit_sprite, 1);
	CAT_prop_init(lantern_lit_item, 1, 1, true);

	bowl_stone_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Stone Bowl", bowl_stone_sprite, 1);
	CAT_prop_init(bowl_stone_item, 1, 1, false);

	bowl_gold_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Gold Bowl", bowl_gold_sprite, 1);
	CAT_prop_init(bowl_gold_item, 1, 1, false);

	pixel_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Pixel", pixel_sprite, 1);
	CAT_prop_init(pixel_item, 1, 1, false);
}
