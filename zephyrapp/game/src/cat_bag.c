#include "cat_bag.h"

#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_actions.h"
#include <string.h>
#include "cat_sprite.h"
#include <stddef.h>
#include "cat_menu.h"

CAT_item_list bag =
{
	.length = 0
};
int coins = 0;

static int base = 0;
static int selector = 0;


static bool prop_filter(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return false;
	return item->type == CAT_ITEM_TYPE_PROP;
}
static bool tool_filter(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return false;
	return item->type == CAT_ITEM_TYPE_TOOL;
}
static bool key_filter(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return false;
	return item->type == CAT_ITEM_TYPE_KEY;
}

struct
{
	const char* title;
	CAT_item_filter filter;
} tabs[] =
{
	{"ALL", NULL},
	{"PROPS", prop_filter},
	{"TOOLS", tool_filter},
	{"KEYS", key_filter}
};
#define NUM_TABS (sizeof(tabs)/sizeof(tabs[0]))
static int tab_selector = 0;

CAT_item_list roster =
{
	.length = 0
};

void CAT_MS_bag(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			roster.length = 0;
			CAT_item_list_filter(&bag, &roster, tabs[tab_selector].filter);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
					CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			
			bool tab_changed = false;
			if(CAT_input_pulse(CAT_BUTTON_LEFT))
			{
				tab_selector -= 1;
				if(tab_selector == -1)
					tab_selector = NUM_TABS-1;	
				tab_changed = true;
			}
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
			{
				tab_selector += 1;
				if(tab_selector == NUM_TABS)
					tab_selector = 0;		
				tab_changed = true;
			}
			if(tab_changed)
			{
				roster.length = 0;
				CAT_item_list_filter(&bag, &roster, tabs[tab_selector].filter);
				base = 0;
				selector = 0;
			}
			
			if(roster.length <= 0)
				break;

			if(CAT_input_pulse(CAT_BUTTON_UP))
			{
				selector -= 1;
				if(selector == -1)
					selector = roster.length-1;		
			}
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
			{
				selector += 1;
				if(selector == roster.length)
					selector = 0;
			}
			selector = clamp(selector, 0, roster.length-1);

			int overshoot = selector - base;
			if(overshoot < 0)
				base += overshoot;
			else if(overshoot >= 9)
				base += (overshoot - 8);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				CAT_machine_transition(CAT_MS_inspector);
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_bag()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_textf("< BAG (%s) > ", tabs[tab_selector].title);
	CAT_gui_image(icon_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	if(roster.length == 0)
	{
		CAT_gui_text("Nothing to see here...\n");
		return;
	}
	for(int i = 0; i < 9; i++)
	{
		int idx = base + i;
		if(idx >= roster.length)
			return;

		int item_id = roster.item_ids[idx];
		CAT_item* item = CAT_item_get(item_id);

		CAT_gui_set_flag(CAT_GUI_TIGHT);
		CAT_gui_panel((CAT_ivec2) {0, 2+i*2}, (CAT_ivec2) {15, 2});
		CAT_gui_image(item->icon_id, 0);
		
		CAT_gui_textf(" %s *%d ", item->name, roster.counts[idx]);

		if(idx == selector)
			CAT_gui_image(icon_pointer_sprite, 0);
	}
}

void CAT_MS_inspector(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_render_inspector()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("ITEM INSPECTOR ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	int item_id = roster.item_ids[selector];
	CAT_item* item = CAT_item_get(item_id);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	CAT_gui_image(item->sprite_id, 0);
	CAT_gui_textf(" %s", item->name);
	CAT_gui_div("");

	if(strlen(item->text) > 0)
	{
		CAT_gui_set_flag(CAT_GUI_WRAP_TEXT);
		CAT_gui_text(item->text);
		CAT_gui_div("");
	}
}