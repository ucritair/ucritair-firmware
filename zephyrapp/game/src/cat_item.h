#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include "cat_math.h"
#include "cat_render.h"
#include "cat_structures.h"
#include "item_assets.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_ITEM_TABLE_CAPACITY 128


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
	uint16_t price;
	const char* text;
	const CAT_sprite* icon;

	union
	{
		struct
		{
			CAT_tool_type type;

			const CAT_sprite* cursor;
			int8_t dv;
			int8_t df;
			int8_t ds;

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
			int8_t child_dy;
		} prop_data;
	} data;
} CAT_item;

typedef struct CAT_item_table
{
	CAT_item data[CAT_ITEM_TABLE_CAPACITY];
	uint16_t counts[CAT_ITEM_TABLE_CAPACITY];
	uint16_t length;
} CAT_item_table;
extern CAT_item_table item_table;

CAT_item* CAT_item_get(int item_id);

typedef bool (*CAT_item_filter)(int item_id);
void CAT_filter_item_table(CAT_item_filter filter, CAT_int_list* list);


