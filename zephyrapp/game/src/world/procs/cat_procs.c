#include "cat_procs.h"

#include "cat_item.h"
#include "item_assets.h"

void dialogue_proc_give_carrot()
{
	CAT_inventory_add(food_carrots_item, 1);
}