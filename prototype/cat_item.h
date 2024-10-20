#ifndef CAT_ITEM_H
#define CAT_ITEM_H

typedef enum CAT_item_type
{
	CAT_ITEM_TYPE_PROP,
	CAT_ITEM_TYPE_FOOD
} CAT_item_type;

typedef struct CAT_item
{
	CAT_item_type type;

	const char* name;
	CAT_anim* anim;
	int price;

	union
	{
		struct
		{
			int tile_width;
			int tile_height;
		} prop_data;

		struct
		{
			float delta_vigor;
			float delta_focus;
			float delta_soul;
		} food_data;
	} data;
} CAT_item;

void CAT_item_init(CAT_item* item, CAT_item_type type, const char* name, CAT_anim* anim)
{
	item->type = type;
	item->name = name;
	item->anim = anim;
}

typedef struct CAT_store
{
	CAT_item* map[256];
} CAT_store;

void CAT_store_add(CAT_store* store, int key, CAT_item* value)
{
	store->map[key] = value;
}

typedef struct CAT_bag
{
	int keys[256];
	int length;	
} CAT_bag;

void CAT_bag_init(CAT_bag* bag)
{
	bag->length = 0;
}

void CAT_bag_add(CAT_bag* bag, int key)
{
	if(bag->length >= 256)
		return;

	bag->keys[bag->length] = key;
	bag->length += 1;
}

int CAT_bag_find(CAT_bag* bag, int key)
{
	for(int i = 0; i < bag->length; i++)
	{
		if(bag->keys[i] == key)
		{
			return i;
		}
	}
	return -1;
}

void CAT_bag_remove(CAT_bag* bag, int key)
{
	int idx = CAT_bag_find(bag, key);
	if(idx == -1)
		return;

	for(int i = bag->length-1; i > idx; i--)
	{
		bag->keys[i-1] = bag->keys[i];
	}
	bag->length -= 1;
}

#endif
