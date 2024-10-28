#include "cat_item.h"

//////////////////////////////////////////////////////////////////////////
// TABLE AND BAG

CAT_item_table item_table;

void CAT_item_table_init()
{
	item_table.length = 0;
}

int CAT_item_init(CAT_item_type type, const char* name, int sprite, int price)
{
	if(item_table.length >= CAT_ITEM_TABLE_MAX_LENGTH)
		return -1;

	int item_id = item_table.length;
	item_table.length += 1;

	CAT_item* item = &item_table.data[item_id];
	item->type = type;
	item->name = name;
	item->sprite = sprite;
	item->price = price;
	item->count = 0;

	return item_id;
}

CAT_item* CAT_item_get(int item_id)
{
	if(item_id < 0 || item_id >= CAT_ITEM_TABLE_MAX_LENGTH)
		return NULL;
	return &item_table.data[item_id];
}

void CAT_prop_init(int item_id, int anim_id, int width, int height)
{
	CAT_item* item = CAT_item_get(item_id);
	item->data.prop_data.anim_id = anim_id;
	item->data.prop_data.shape = (CAT_ivec2) {width, height};
}

void CAT_food_init(int item_id, float d_v, float d_f, float d_s, float dd_v, float dd_f, float dd_s)
{
	CAT_item* item = CAT_item_get(item_id);
	item->data.food_data.d_v = d_v;
	item->data.food_data.d_f = d_f;
	item->data.food_data.d_s = d_s;
	item->data.food_data.dd_v = dd_v;
	item->data.food_data.dd_f = dd_f;
	item->data.food_data.dd_s = dd_s;
}

void CAT_bag_add(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	item->count += 1;
}

bool CAT_bag_remove(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	if(item->count <= 0)
		return false;
	item->count -= 1;
	return true;
}

int CAT_bag_count(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	return item->count;
}

int CAT_bag_seek(int whence, CAT_item_type type)
{
	int idx = whence;
	for(int steps = 0; steps < item_table.length; steps++)
	{
		CAT_item* item = CAT_item_get(idx);
		if(item == NULL || item->count <= 0)
			continue;

		if(item->type == type)
			return idx;

		idx += 1;
		if(idx >= item_table.length)
			idx = 0;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

int chair_item_id;
int table_item_id;
int coffee_item_id;
int device_item_id;

void CAT_item_mass_define()
{
	chair_item_id = CAT_item_init(CAT_ITEM_TYPE_PROP, "Chair", chair_sprite_id[0], 1);
	CAT_prop_init(chair_item_id, chair_anim_id, 2, 2);
	CAT_bag_add(chair_item_id);

	table_item_id = CAT_item_init(CAT_ITEM_TYPE_PROP, "Table", table_sprite_id, 1);
	CAT_prop_init(table_item_id, -1, 4, 2);
	CAT_bag_add(table_item_id);
	
	coffee_item_id = CAT_item_init(CAT_ITEM_TYPE_PROP, "Coffee", coffee_sprite_id[0], 1);
	CAT_prop_init(coffee_item_id, coffee_anim_id, 4, 2);
	CAT_bag_add(coffee_item_id);
	
	device_item_id = CAT_item_init(CAT_ITEM_TYPE_PROP, "Device", device_sprite_id, 1);
	CAT_prop_init(device_item_id, -1, 4, 2);
	CAT_bag_add(device_item_id);
}
