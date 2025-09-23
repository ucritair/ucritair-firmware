#include "cat_procs.h"

#include "cat_item.h"
#include "item_assets.h"
#include "cat_dialogue.h"
#include "dialogue_assets.h"
#include "cat_math.h"
#include "cat_machine.h"
#include "cat_room.h"
#include "dialogue_profile_assets.h"
#include "prop_assets.h"

void interact_proc_reed(CAT_prop_instance* prop)
{
	CAT_activate_dialogue_profile(&dialogue_profile_reed);
	CAT_enter_dialogue(CAT_poll_dialogue_profile());
}

void interact_proc_reed_house(CAT_prop_instance* prop)
{
	CAT_enter_dialogue(&dialogue_reed_house);
}

void interact_proc_statue(CAT_prop_instance* prop)
{
	CAT_activate_dialogue_profile(&dialogue_profile_bird_statue);
	CAT_enter_dialogue(CAT_poll_dialogue_profile());
}

void interact_proc_market(CAT_prop_instance* prop)
{
	CAT_enter_dialogue(&dialogue_market_default);
}

void interact_proc_arcade(CAT_prop_instance* prop)
{
	CAT_enter_dialogue(&dialogue_arcade_default);
}

CAT_item_pool item_pool = (CAT_item_pool)
{
	.entries = (struct item_pool_entry[])
	{
		{
			.item_id = fish_grade_0_item,
			.weight = 1
		},
		{
			.item_id = fish_grade_1_item,
			.weight = 2
		},
		{
			.item_id = fish_grade_2_item,
			.weight = 3
		},
	},
	.entry_count = 3,
};

void proc_collect_resource(CAT_prop_instance* prop)
{
	if(prop->prop == &node_grass_prop)
		CAT_inventory_add(ingr_fibre_item, 1);
	if(prop->prop == &node_stone_prop)
		CAT_inventory_add(ingr_stone_item, 1);
	if(prop->prop == &node_wood_prop)
		CAT_inventory_add(ingr_wood_item, 1);
	prop->disabled = true;
}

void proc_coc_innerworld(CAT_prop_instance* prop)
{
	CAT_pushdown_rebase(CAT_MS_room);
}

