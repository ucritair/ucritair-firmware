#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include "cat_math.h"
#include "cat_render.h"
#include "cat_structures.h"
#include "cat_machine.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_ITEM_TABLE_CAPACITY 256


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

typedef enum
{
	CAT_PROP_TYPE_DEFAULT,
	CAT_PROP_TYPE_BOTTOM,
	CAT_PROP_TYPE_TOP
} CAT_prop_type;

typedef struct
{
	CAT_item_type type;

	const char* name;
	const CAT_sprite* sprite;
	uint16_t price;
	const char* text;
	bool can_buy;
	bool can_sell;

	union
	{
		struct
		{
			CAT_tool_type tool_type;

			const CAT_sprite* tool_cursor;
			int8_t tool_dv;
			int8_t tool_df;
			int8_t tool_ds;

			union
			{
				struct
				{
					CAT_food_group food_group;
					CAT_food_role food_role;
				};
			};
		};

		struct
		{
			CAT_prop_type prop_type;
			CAT_ivec2 prop_shape;
			bool prop_animated;
			int8_t prop_child_dy;
		};
	};
} CAT_item;

#define NULL_ITEM -1

typedef struct
{
	CAT_item data[CAT_ITEM_TABLE_CAPACITY];
	uint16_t counts[CAT_ITEM_TABLE_CAPACITY];
	uint16_t length;
} CAT_item_table;
extern CAT_item_table item_table;
CAT_item* CAT_get_item(int item_id);

typedef bool (*CAT_item_filter)(int item_id);

typedef struct
{
	int item;
	uint16_t count;
} CAT_item_bundle;


//////////////////////////////////////////////////////////////////////////
// BAG

void CAT_inventory_clear();
bool CAT_inventory_add(int item_id, int count);
bool CAT_inventory_remove(int item_id, int count);
int CAT_inventory_count(int item_id);


//////////////////////////////////////////////////////////////////////////
// INVENTORY

void CAT_bind_inspector(int item_id);
void CAT_MS_inspector(CAT_FSM_signal signal);
void CAT_render_inspector();

void CAT_MS_inventory(CAT_FSM_signal signal);
void CAT_render_inventory();


//////////////////////////////////////////////////////////////////////////
// SHOP

void CAT_bind_checkout(int item_id);
void CAT_MS_checkout(CAT_FSM_signal signal);
void CAT_render_checkout();

void CAT_bind_sale(int item_id);
void CAT_MS_sale(CAT_FSM_signal signal);
void CAT_render_sale();

void CAT_MS_shop(CAT_FSM_signal signal);
void CAT_render_shop();

