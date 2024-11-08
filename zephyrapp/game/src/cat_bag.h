#ifndef CAT_BAG_H
#define CAT_BAG_H

#include "cat_machine.h"

#define CAT_BAG_MAX_LENGTH 256

typedef struct CAT_bag
{
	int item_id[CAT_BAG_MAX_LENGTH];
	int count[CAT_BAG_MAX_LENGTH];
	int length;

	int coins;
} CAT_bag;
extern CAT_bag bag;

void CAT_bag_init();
int CAT_bag_find(int item_id);
void CAT_bag_add(int item_id);
void CAT_bag_remove(int item_id);

typedef struct CAT_bag_state
{
	int base;
	int idx;
	CAT_machine_state destination;
} CAT_bag_state;
extern CAT_bag_state bag_state;

void CAT_bag_state_init();

#endif