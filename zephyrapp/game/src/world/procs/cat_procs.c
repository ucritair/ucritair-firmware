#include "cat_procs.h"

#include "cat_item.h"
#include "item_assets.h"
#include "cat_dialogue.h"
#include "dialogue_assets.h"

void interact_proc_reed()
{
	CAT_enter_dialogue(&dialogue_test_a);
}

void dialogue_proc_give_carrot()
{
	CAT_inventory_add(food_carrots_item, 1);
}