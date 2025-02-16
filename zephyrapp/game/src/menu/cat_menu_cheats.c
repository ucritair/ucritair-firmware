#include "cat_menu.h"

#include "cat_bag.h"
#include "cat_pet.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_main.h"

void cheat_proc_money()
{
	coins += 1000;
}

static CAT_menu_node money_cheat =
{
	.title = "INSTANT CASH",
	.proc = cheat_proc_money,
	.state = NULL,
	.selector = 0,
	.children = { NULL },
};

void cheat_proc_base_stats()
{
	pet.vigour = 9;
	pet.focus = 9;
	pet.spirit = 9;
	CAT_pet_reanimate();
}

static CAT_menu_node base_stats_cheat =
{
	.title = "BASE STATS",
	.proc = cheat_proc_base_stats,
	.state = NULL,
	.selector = 0,
	.children = { NULL },
};

void cheat_proc_crit_stats()
{
	pet.vigour = 3;
	pet.focus = 3;
	pet.spirit = 3;
	CAT_pet_reanimate();
}

static CAT_menu_node crit_stats_cheat =
{
	.title = "CRITICAL STATS",
	.proc = cheat_proc_crit_stats,
	.state = NULL,
	.selector = 0,
	.children = { NULL },
};

void cheat_proc_items()
{
	for(int item_id = 0; item_id < item_table.length; item_id++)
	{
		CAT_item_list_add(&bag, item_id, 1);
	}
}

static CAT_menu_node items_cheat =
{
	.title = "EVERY ITEM",
	.proc = cheat_proc_items,
	.state = NULL,
	.selector = 0,
	.children = { NULL },
};

void cheat_proc_turnkey_apartment()
{
	needs_load = true;
	override_load = true;
}

static CAT_menu_node turnkey_apartment_cheat =
{
	.title = "TURNKEY APARTMENT",
	.proc = cheat_proc_turnkey_apartment,
	.state = NULL,
	.selector = 0,
	.children = { NULL },
};

CAT_menu_node menu_node_cheats =
{
	.title = "CHEATS",
	.proc = NULL,
	.state = NULL,
	.selector = 0,
	.children =
	{
		&money_cheat,
		&items_cheat,
		&base_stats_cheat,
		&crit_stats_cheat,
		&turnkey_apartment_cheat,
		NULL
	},
};