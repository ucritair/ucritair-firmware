#ifndef CAT_ITEM_H
#define CAT_ITEM_H

#include <stdbool.h>
#include <stdlib.h>

#include "cat_sprite.h"
#include "cat_math.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_ITEM_TABLE_MAX_LENGTH 64

//////////////////////////////////////////////////////////////////////////
// TABLE AND BAG

typedef enum CAT_item_type
{
	CAT_ITEM_TYPE_KEY,
	CAT_ITEM_TYPE_PROP,
	CAT_ITEM_TYPE_FOOD
} CAT_item_type;

typedef struct CAT_item
{
	CAT_item_type type;
	const char* name;
	int sprite; 
	int price;
	int count;

	union
	{
		struct
		{
			int anim_id;
			CAT_ivec2 shape;
		} prop_data;

		struct
		{
			float d_v;
			float d_f;
			float d_s;
			float dd_v;
			float dd_f;
			float dd_s;
		} food_data;
	} data;
} CAT_item;

typedef struct CAT_item_table
{
	CAT_item data[CAT_ITEM_TABLE_MAX_LENGTH];
	int length;
} CAT_item_table;
extern CAT_item_table item_table;

void CAT_item_table_init();
int CAT_item_init(CAT_item_type type, const char* name, int sprite, int price);
CAT_item* CAT_item_get(int item_id);
void CAT_prop_init(int item_id, int anim_id, int width, int height);
void CAT_food_init(int item_id, float d_v, float d_f, float d_s, float dd_v, float dd_f, float dd_s);

void CAT_bag_add(int item_id);
bool CAT_bag_remove(int item_id);
int CAT_bag_count(int item_id);
int CAT_bag_seek(int whence, CAT_item_type type);

//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

extern int chair_item_id;
extern int table_item_id;
extern int coffee_item_id;
extern int device_item_id;
extern int seed_item_id;

void CAT_item_mass_define();

#endif
