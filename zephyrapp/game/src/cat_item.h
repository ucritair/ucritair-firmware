#ifndef CAT_ITEM_H
#define CAT_ITEM_H

#include <stdbool.h>
#include <stdlib.h>

#include "cat_math.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_ITEM_TABLE_MAX_LENGTH 64


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
			float d_v;
			float d_f;
			float d_s;
		} food_data;

		struct
		{
			int icon_sprite_id;
		} book_data;

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

void CAT_food_init(int item_id, float d_v, float d_f, float d_s);
void CAT_book_init(int item_id, int icon_sprite_id);
void CAT_prop_init(int item_id, int width, int height, bool animate);
void CAT_gear_init(int item_id);
void CAT_gear_toggle(int item_id, bool equipped);
bool CAT_gear_status(int item_id);


//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

extern int cigarettes_item;
extern int sausage_item;
extern int padkrapow_item;
extern int book_a_item;
extern int book_b_item;
extern int book_c_item;
extern int toy_a_item;
extern int toy_b_item;
extern int toy_c_item;
extern int mask_item;

extern int solderpaste_item;
extern int coffee_item;
extern int fan_item;
extern int purifier_item;
extern int uv_item;
extern int crypto_item;

extern int chair_wood_item;
extern int table_sm_item;
extern int table_lg_item;

extern int lantern_lit_item;
extern int lantern_unlit_item;
extern int bowl_stone_item;
extern int bowl_gold_item;

extern int pixel_item;

void CAT_item_mass_define();

#endif
