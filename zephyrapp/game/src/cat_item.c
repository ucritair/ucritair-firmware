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

int CAT_item_init(CAT_item_type type, const char* name, int sprite_id)
{
	int item_id = item_table.length;
	item_table.length += 1;

	CAT_item* item = &item_table.data[item_id];
	item->type = type;
	item->name = name;
	item->sprite_id = sprite_id;

	return item_id;
}

CAT_item* CAT_item_get(int item_id)
{
	return &item_table.data[item_id];
}

void CAT_prop_init(int item_id, int width, int height, bool animate)
{
	CAT_item* item = CAT_item_get(item_id);
	item->data.prop_data.shape = (CAT_ivec2) {width, height};
	item->data.prop_data.animate = animate;
	item->data.prop_data.frame_idx = 0;
}

void CAT_prop_flip(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	if(item->data.prop_data.animate)
		return;
	int* frame_idx = &item->data.prop_data.frame_idx;
	*frame_idx += 1;
	int sprite_id = item->sprite_id;
	CAT_sprite* sprite = &atlas.data[sprite_id];
	if(*frame_idx >= sprite->frame_count)
		*frame_idx = 0;
}

void CAT_food_init(int item_id, float d_v, float d_f, float d_s)
{
	CAT_item* item = CAT_item_get(item_id);
	item->data.food_data.d_v = d_v;
	item->data.food_data.d_f = d_f;
	item->data.food_data.d_s = d_s;
}


//////////////////////////////////////////////////////////////////////////
// BAG

CAT_bag bag;

void CAT_bag_init()
{
	bag.length = 0;
}

int CAT_bag_find(int item_id)
{
	for(int i = 0; i < bag.length; i++)
	{
		if(bag.item_id[i] == item_id)
		{
			return i;
		}
	}
	return -1;
}

void CAT_bag_add(int item_id)
{
	int idx = CAT_bag_find(item_id);
	if(idx >= 0)
	{
		bag.count[idx] += 1;
	}
	else
	{
		bag.item_id[bag.length] = item_id;
		bag.count[bag.length] = 1;
		bag.length += 1;
	}
}

void CAT_bag_remove(int item_id)
{
	int idx = CAT_bag_find(item_id);
	if(idx >= 0)
	{
		bag.count[idx] -= 1;
		if(bag.count[idx] <= 0)
		{
			for(int i = idx; i < bag.length-1; i++)
			{
				bag.item_id[i] = bag.item_id[i+1];
				bag.count[i] = bag.item_id[i+1];
			}
			bag.length -= 1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

int solderpaste_item;
int coffeemaker_item;
int fan_item;
int purifier_item;

int chair_wood_item;
int table_sm_item;
int table_lg_item;
int stool_wood_item;
int stool_stone_item;
int stool_gold_item;

int lantern_lit_item;
int lantern_unlit_item;
int bowl_stone_item;
int bowl_gold_item;
int vase_stone_item;
int vase_gold_item;
int flower_empty_item;
int flower_vig_item;
int flower_foc_item;
int flower_spi_item;
int plant_green_item;
int plant_maroon_item;
int plant_purple_item;
int plant_yellow_item;
int bush_plain_item;
int bush_daisy_item;
int bush_lilac_item;
int succulent_item;

int crystal_blue_sm_item;
int crystal_green_sm_item;
int crystal_purple_sm_item;
int crystal_blue_hrt_item;
int crystal_green_hrt_item;
int crystal_purple_hrt_item;
int crystal_blue_md_item;
int crystal_green_md_item;
int crystal_purple_md_item;
int crystal_blue_lg_item;
int crystal_green_lg_item;
int crystal_purple_lg_item;

int cigarettes_item;
int sausage_item;
int padkrapow_item;
int coffee_item;

void CAT_item_mass_define()
{
	purifier_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Air Purifier", purifier_sprite);
	CAT_prop_init(purifier_item, 2, 1, true);
	CAT_bag_add(purifier_item);

	chair_wood_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Wood Chair", chair_wood_sprite);
	CAT_prop_init(chair_wood_item, 2, 2, false);
	CAT_bag_add(chair_wood_item);

	table_sm_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Small Table", table_sm_sprite);
	CAT_prop_init(table_sm_item, 2, 2, false);
	CAT_bag_add(table_sm_item);

	table_lg_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Large Table", table_lg_sprite);
	CAT_prop_init(table_lg_item, 4, 2, false);
	CAT_bag_add(table_lg_item);

	coffeemaker_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Coffee Machine", coffeemaker_sprite);
	CAT_prop_init(coffeemaker_item, 2, 2, true);
	CAT_bag_add(coffeemaker_item);

	fan_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Fan", fan_sprite);
	CAT_prop_init(fan_item, 2, 1, true);
	CAT_bag_add(fan_item);

	lantern_lit_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Lantern (Lit)", lantern_lit_sprite);
	CAT_prop_init(lantern_lit_item, 1, 1, true);
	CAT_bag_add(lantern_lit_item);

	lantern_unlit_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Lantern (Unlit)", lantern_unlit_sprite);
	CAT_prop_init(lantern_unlit_item, 1, 1, false);
	CAT_bag_add(lantern_unlit_item);

	bowl_stone_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Stone Bowl", bowl_stone_sprite);
	CAT_prop_init(bowl_stone_item, 1, 1, false);
	CAT_bag_add(bowl_stone_item);

	bowl_gold_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Gold Bowl", bowl_gold_sprite);
	CAT_prop_init(bowl_gold_item, 1, 1, false);
	CAT_bag_add(bowl_gold_item);

	vase_stone_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Stone Vase", vase_stone_sprite);
	CAT_prop_init(vase_stone_item, 1, 1, false);
	CAT_bag_add(vase_stone_item);

	vase_gold_item = CAT_item_init(CAT_ITEM_TYPE_PROP, "Gold Vase", vase_gold_sprite);
	CAT_prop_init(vase_gold_item, 1, 1, false);
	CAT_bag_add(vase_gold_item);

	cigarettes_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Cigarettes", cigarette_sprite);
	CAT_food_init(cigarettes_item, 0, 0, 1);
	CAT_bag_add(cigarettes_item);

	sausage_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Sausage", sausage_sprite);
	CAT_food_init(sausage_item, 1, 0, 0);
	CAT_bag_add(sausage_item);

	padkrapow_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Pad Kra Pow", padkrapow_sprite);
	CAT_food_init(padkrapow_item, 1, 0, 0);
	CAT_bag_add(padkrapow_item);

	coffee_item = CAT_item_init(CAT_ITEM_TYPE_FOOD, "Coffee", coffee_sprite);
	CAT_food_init(coffee_item, 0, 1, 0);
	CAT_bag_add(coffee_item);
}
