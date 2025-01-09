#include "cat_menu.h"

#include "cat_bag.h"
#include "cat_pet.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_room.h"
#include "theme_assets.h"

void theme_proc_base()
{
	room.theme = &base_theme;
}

static CAT_menu_node menu_node_base_theme =
{
	.title = "BASIC THEME",
	.proc = theme_proc_base,
	.state = NULL,
	.children = { NULL },
};

void theme_proc_ash()
{
	room.theme = &ash_theme;
}

static CAT_menu_node menu_node_ash_theme =
{
	.title = "ASHEN THEME",
	.proc = theme_proc_ash,
	.state = NULL,
	.children = { NULL },
};

void theme_proc_grass()
{
	room.theme = &grass_theme;
}

static CAT_menu_node menu_node_grass_theme =
{
	.title = "VERDANT THEME",
	.proc = theme_proc_grass,
	.state = NULL,
	.children = { NULL },
};

CAT_menu_node menu_node_themes =
{
	.title = "ROOM THEME",
	.proc = NULL,
	.state = NULL,
	.children =
	{
		&menu_node_base_theme,
		&menu_node_ash_theme,
		&menu_node_grass_theme,
		NULL
	},	
};