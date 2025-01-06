#include "cat_menu.h"

#include "cat_room.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"
#include "cat_sprite.h"
#include "cat_version.h"
#include "cat_manual.h"
#include "cat_vending.h"
#include "cat_insights.h"
#include "cat_arcade.h"
#include "cat_bag.h"
#include "cat_pet.h"
#include <stddef.h>
#include <string.h>

#ifdef CAT_EMBEDDED
#include "menu_system.h"
#include "menu_aqi.h"
#endif

static CAT_menu_node insights =
{
	.title = "INSIGHTS",
	.children = { NULL },
	.state = CAT_MS_insights
};

static CAT_menu_node name =
{
	.title = "PET NAME",
	.children = { NULL },
	.state = CAT_MS_menu
};

static CAT_menu_node settings =
{
	.title = "SETTINGS",
	.children =
	{
		&name,
		NULL
	},
	.state = NULL
};

static CAT_menu_node bage =
{
	.title = "BAG",
	.children = { NULL },
	.state = CAT_MS_bag
};

static CAT_menu_node vending =
{
	.title = "VENDING MACHINE",
	.children = { NULL },
	.state = CAT_MS_vending
};

static CAT_menu_node arcade =
{
	.title = "ARCADE CABINET",
	.children = { NULL },
	.state = CAT_MS_arcade
};

#ifdef CAT_EMBEDDED
static CAT_menu_node air =
{
	.title = "AIR QUALITY",
	.children = { NULL },
	.state = CAT_MS_aqi
};

static CAT_menu_node system =
{
	.title = "SYSTEM MENU",
	.children = { NULL },
	.state = CAT_MS_system_menu
};
#endif

static CAT_menu_node magic =
{
	.title = "MAGIC",
	.children = { NULL },
	.state = CAT_MS_magic
};

static CAT_menu_node manual =
{
	.title = "MANUAL",
	.children = { NULL },
	.state = CAT_MS_manual
};

static CAT_menu_node debug =
{
	.title = "DEBUG",
	.children = { NULL },
	.state = CAT_MS_debug
};

extern CAT_menu_node cheats;

static CAT_menu_node root =
{
	.title = "MENU",
	.children =
	{
		&insights,
		&settings,
		&bage,
		&vending,
		&arcade,
#ifdef CAT_EMBEDDED
		&air,
		&system,
#endif
		&magic,
		&manual,
		&debug,
		&cheats,
		NULL
	},
	.state = NULL
};

static CAT_menu_node* stack[16];
static int stack_length = 0;

void push(CAT_menu_node* node)
{
	stack[stack_length] = node;
	stack_length += 1;
}

CAT_menu_node* pop()
{
	stack_length -= 1;
	return stack[stack_length];
}

static int selector = 0;

void CAT_MS_menu(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			stack_length = 0;
			push(&root);
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				pop();
				if(stack_length == 0)
				{
					CAT_machine_back();
					break;
				}
			}
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			
			CAT_menu_node* node = stack[stack_length-1];
			int child_count = 0;
			for(int i = 0; node->children[i] != NULL; i++)
				child_count += 1;

			if(CAT_input_pulse(CAT_BUTTON_UP))
				selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				selector += 1;
			selector = clamp(selector, 0, child_count-1);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(child_count > 0)
				{
					CAT_menu_node* child = node->children[selector];
					if(child->state != NULL)
					{
						CAT_machine_transition(child->state);
					}
					else
					{
						push(child);
					}
				}
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_menu()
{
	CAT_menu_node* node = stack[stack_length-1];

	CAT_gui_title
	(
		false,
		&icon_enter_sprite, &icon_exit_sprite,
		node->title
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	for(int i = 0; node->children[i] != NULL; i++)
	{
		CAT_gui_textf("\1 %s ", node->children[i]->title);
		if(i == selector)
			CAT_gui_image(&icon_pointer_sprite, 0);
		CAT_gui_line_break();
	}
}
