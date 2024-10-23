#include "cat_item.h"

void CAT_item_init(CAT_item* item, CAT_item_type type, const char* name, int icon, CAT_anim* anim)
{
	item->type = type;
	item->name = name;
	item->icon = icon;
	item->anim = anim;
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
