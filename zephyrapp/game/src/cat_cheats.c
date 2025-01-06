#include "cat_menu.h"
#include "cat_bag.h"
#include "cat_pet.h"
#include "cat_gui.h"
#include "cat_input.h"

void cheat_proc_money(CAT_machine_signal signal)
{
	coins += 1000;
	CAT_machine_back();
}

static CAT_menu_node money_cheat =
{
	.title = "INSTANT CASH",
	.children = { NULL },
	.state = cheat_proc_money
};

void cheat_proc_base_stats(CAT_machine_signal signal)
{
	pet.vigour = 9;
	pet.focus = 9;
	pet.spirit = 9;
	CAT_pet_reanimate();
	CAT_machine_back();
}

static CAT_menu_node base_stats_cheat =
{
	.title = "BASE STATS",
	.children = { NULL },
	.state = cheat_proc_base_stats
};

void cheat_proc_crit_stats(CAT_machine_signal signal)
{
	pet.vigour = 3;
	pet.focus = 3;
	pet.spirit = 3;
	CAT_pet_reanimate();
	CAT_machine_back();
}

static CAT_menu_node crit_stats_cheat =
{
	.title = "CRITICAL STATS",
	.children = { NULL },
	.state = cheat_proc_crit_stats
};

void cheat_proc_items(CAT_machine_signal signal)
{
	for(int item_id = 0; item_id < item_table.length; item_id++)
	{
		CAT_item_list_add(&bag, item_id, 1);
	}
	CAT_machine_back();
}

static CAT_menu_node items_cheat =
{
	.title = "EVERY ITEM",
	.children = { NULL },
	.state = cheat_proc_items
};

CAT_menu_node cheats =
{
	.title = "CHEATS",
	.children =
	{
		&money_cheat,
		&items_cheat,
		&base_stats_cheat,
		&crit_stats_cheat,
		NULL
	},
	.state = NULL
};