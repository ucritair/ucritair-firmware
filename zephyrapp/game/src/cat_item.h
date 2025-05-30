#pragma once

#include "../data/item_assets.h"

#include <stdbool.h>
#include <stdlib.h>
#include "cat_math.h"
#include "cat_render.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_ITEM_TABLE_MAX_LENGTH 128
#define CAT_ITEM_LIST_MAX_LENGTH CAT_ITEM_TABLE_MAX_LENGTH


//////////////////////////////////////////////////////////////////////////
// ITEM TABLE

typedef enum
{
	CAT_ITEM_TYPE_KEY,
	CAT_ITEM_TYPE_TOOL,
	CAT_ITEM_TYPE_PROP
} CAT_item_type;

typedef enum
{
	CAT_TOOL_TYPE_FOOD,
	CAT_TOOL_TYPE_BOOK,
	CAT_TOOL_TYPE_TOY
} CAT_tool_type;

typedef enum
{
	CAT_FOOD_GROUP_VEG,
	CAT_FOOD_GROUP_STARCH,
	CAT_FOOD_GROUP_MEAT,
	CAT_FOOD_GROUP_DAIRY,
	CAT_FOOD_GROUP_MISC
} CAT_food_group;

typedef enum
{
	CAT_FOOD_ROLE_STAPLE,
	CAT_FOOD_ROLE_MAIN,
	CAT_FOOD_ROLE_SIDE,
	CAT_FOOD_ROLE_SOUP,
	CAT_FOOD_ROLE_DRINK,
	CAT_FOOD_ROLE_TREAT,
	CAT_FOOD_ROLE_VICE
} CAT_food_role;

typedef enum CAT_prop_type
{
	CAT_PROP_TYPE_DEFAULT,
	CAT_PROP_TYPE_BOTTOM,
	CAT_PROP_TYPE_TOP
} CAT_prop_type;

typedef struct CAT_item
{
	CAT_item_type type;

	const char* name;
	const CAT_sprite* sprite;
	int price;
	const char* text;
	const CAT_sprite* icon;

	union
	{
		struct
		{
			CAT_tool_type type;

			const CAT_sprite* cursor;
			int dv;
			int df;
			int ds;

			union
			{
				struct
				{
					CAT_food_group food_group;
					CAT_food_role food_role;
				};
			};
		} tool_data;

		struct
		{
			CAT_prop_type type;
			CAT_ivec2 shape;
			bool animate;
			int child_dy;
		} prop_data;
	} data;
} CAT_item;

typedef struct CAT_item_table
{
	CAT_item data[CAT_ITEM_TABLE_MAX_LENGTH];
	int length;
} CAT_item_table;
extern CAT_item_table item_table;

CAT_item* CAT_item_get(int item_id);


//////////////////////////////////////////////////////////////////////////
// ITEM LIST

typedef struct CAT_item_list
{
	int item_ids[CAT_ITEM_LIST_MAX_LENGTH];
	int counts[CAT_ITEM_LIST_MAX_LENGTH];
	int length;
} CAT_item_list;

typedef bool (*CAT_item_filter)(int item_id);

void CAT_item_list_init(CAT_item_list* item_list);
int CAT_item_list_find(CAT_item_list* item_list, int item_id);
int CAT_item_list_add(CAT_item_list* item_list, int item_id, int count);
void CAT_item_list_remove(CAT_item_list* item_list, int item_id, int count);
void CAT_item_list_filter(CAT_item_list* a, CAT_item_list* b, CAT_item_filter filter);
