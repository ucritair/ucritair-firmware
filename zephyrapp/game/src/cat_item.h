#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "cat_math.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_ITEM_TABLE_MAX_LENGTH 256
#define CAT_ITEM_LIST_MAX_LENGTH 256


//////////////////////////////////////////////////////////////////////////
// ITEM TABLE

typedef enum CAT_item_type
{
	CAT_ITEM_TYPE_KEY,
	CAT_ITEM_TYPE_FOOD,
	CAT_ITEM_TYPE_BOOK,
	CAT_ITEM_TYPE_TOY,
	CAT_ITEM_TYPE_PROP,
	CAT_ITEM_TYPE_GEAR
} CAT_item_type;

typedef struct CAT_item
{
	CAT_item_type type;
	const char* name;
	int sprite_id;
	int price;

	union
	{
		struct
		{
			int cursor_sprite_id;
			int dv;
			int df;
			int ds;
			bool consumable;
		} tool_data;

		struct
		{
			CAT_ivec2 shape;
			bool animate;
		} prop_data;

		struct
		{
			bool equipped;
		} gear_data;	
	} data;
} CAT_item;

typedef struct CAT_item_table
{
	CAT_item data[CAT_ITEM_TABLE_MAX_LENGTH];
	int length;
} CAT_item_table;
extern CAT_item_table item_table;

void CAT_item_table_init();
int CAT_item_init(CAT_item_type type, const char* name, int sprite_id, int price);
CAT_item* CAT_item_get(int item_id);

void CAT_tool_init(int item_id, int cursor_sprite_id, int dv, int df, int ds, bool consumable);
void CAT_prop_init(int item_id, int width, int height, bool animate);
void CAT_gear_init(int item_id);
void CAT_gear_toggle(int item_id, bool equipped);
bool CAT_gear_status(int item_id);

//////////////////////////////////////////////////////////////////////////
// ITEM LIST

typedef struct CAT_item_list
{
	int item_ids[CAT_ITEM_LIST_MAX_LENGTH];
	int counts[CAT_ITEM_LIST_MAX_LENGTH];
	int length;
} CAT_item_list;

int CAT_item_list_find(CAT_item_list* item_list, int item_id);
void CAT_item_list_add(CAT_item_list* item_list, int item_id);
void CAT_item_list_remove(CAT_item_list* item_list, int item_id);

//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

// TOOLS
extern int padkaprow_item;
extern int sausage_item;
extern int coffee_item;
extern int salad_item;
extern int pill_vig_item;
extern int pill_foc_item;
extern int pill_spi_item;
extern int cigarette_item;

extern int book_a_item;
extern int book_b_item;
extern int book_c_item;
extern int book_d_item;
extern int book_e_item;
extern int book_f_item;

extern int toy_duck_item;
extern int toy_baseball_item;
extern int toy_basketball_item;
extern int toy_golf_item;
extern int toy_puzzle_item;

// KEYS AND GEAR
extern int mask_item;

// PROPS
extern int gpu_item;
extern int uv_item;
extern int purifier_item;

extern int coffeemaker_item;
extern int fan_a_item;
extern int fan_b_item;
extern int lantern_item;

extern int table_lg_item;
extern int table_sm_item;
extern int chair_wood_item;
extern int stool_wood_item;
extern int stool_stone_item;
extern int stool_gold_item;

extern int bowl_stone_item;
extern int bowl_gold_item;
extern int vase_stone_item;
extern int vase_gold_item;

extern int succulent_item;
extern int bush_plain_item;
extern int bush_daisy_item;
extern int bush_lilac_item;
extern int plant_green_item;
extern int plant_maroon_item;
extern int plant_purple_item;
extern int plant_yellow_item;
extern int flower_vig_item;
extern int flower_foc_item;
extern int flower_spi_item;

extern int crystal_blue_lg_item;
extern int crystal_green_lg_item;
extern int crystal_purple_lg_item;

extern int effigy_blue_item;
extern int effigy_purple_item;
extern int effigy_sea_item;

extern int pixel_item;
extern int padkaprop_item;

void CAT_item_mass_define();

