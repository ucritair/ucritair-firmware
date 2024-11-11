#pragma once

#include "cat_machine.h"
#include "cat_item.h"

#define CAT_BAG_MAX_LENGTH CAT_ITEM_TABLE_MAX_LENGTH

typedef struct CAT_bag
{
	int item_ids[CAT_BAG_MAX_LENGTH];
	int counts[CAT_BAG_MAX_LENGTH];
	int length;

	int coins;
} CAT_bag;
extern CAT_bag bag;

int CAT_bag_find(int item_id);
void CAT_bag_add(int item_id);
void CAT_bag_remove(int item_id);

extern CAT_machine_state bag_anchor;
void CAT_render_bag();