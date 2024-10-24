#include "cat_item.h"

void CAT_item_init(CAT_item* item, CAT_item_type type, const char* name, int sprite, int price)
{
	item->type = type;
	item->name = name;
	item->sprite = sprite;
	item->price = price;
}

void CAT_prop_init(CAT_item* item, CAT_anim* anim, int width, int height)
{
	item->data.prop_data.anim = anim;
	item->data.prop_data.width = width;
	item->data.prop_data.height = width;
}

void CAT_food_init(CAT_item* item, float d_v, float d_f, float d_s, float dd_v, float dd_f, float dd_s)
{
	item->data.food_data.d_v = d_v;
	item->data.food_data.d_f = d_f;
	item->data.food_data.d_s = d_s;
	item->data.food_data.dd_v = dd_v;
	item->data.food_data.dd_f = dd_f;
	item->data.food_data.dd_s = dd_s;
}

CAT_store store;

void CAT_store_init()
{
	store.length = 0;
}

int CAT_store_add(CAT_item* value)
{
	if(store.length >= 256)
	{
		return -1;
	}

	int idx = store.length;
	store.table[idx] = value;
	store.length += 1;
	return idx;
}

CAT_bag bag;

void CAT_bag_init()
{
	for(int i = 0; i < 256; i++)
	{
		bag.quantities[i] = 0;
	}
}

void CAT_bag_add(int key)
{
	bag.quantities[key] += 1;
}

void CAT_bag_remove(int key)
{
	bag.quantities[key] -= 1;
}

int CAT_bag_count(int key)
{
	return bag.quantities[key];
}

int CAT_bag_seek(int whence, CAT_item_type type)
{
	int idx = whence;
	for(int steps = 0; steps < 256; steps++)
	{
		if(bag.quantities[idx] <= 0)
			continue;

		CAT_item* item = store.table[idx];
		if(item->type == type)
			return idx;

		idx += 1;
		if(idx >= 256)
			idx = 0;
	}
	return -1;
}
