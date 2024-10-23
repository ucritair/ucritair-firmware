#ifndef CAT_ITEM_H
#define CAT_ITEM_H

#include "cat_sprite.h"

typedef enum CAT_item_type
{
	CAT_ITEM_TYPE_PROP,
	CAT_ITEM_TYPE_FOOD
} CAT_item_type;

typedef struct CAT_item
{
	CAT_item_type type;

	const char* name;
	int icon;
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

void CAT_item_init(CAT_item* item, CAT_item_type type, const char* name, int icon, CAT_anim* anim);

typedef struct CAT_store
{
	CAT_item* table[256];
	int length;
} CAT_store;
extern CAT_store store;

void CAT_store_init();
int CAT_store_add(CAT_item* value);

typedef struct CAT_bag
{
	int quantities[256];
} CAT_bag;
extern CAT_bag bag;

void CAT_bag_init();
void CAT_bag_add(int key);
void CAT_bag_remove(int key);
int CAT_bag_count(int key);

#endif
