#include "cat_inventory.h"

#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include <string.h>
#include "cat_render.h"
#include <stddef.h>
#include "cat_menu.h"
#include "sprite_assets.h"

int coins = 0;

void CAT_bag_clear()
{
	for(int i = 0; i < CAT_ITEM_TABLE_CAPACITY; i++)
	{
		item_table.counts[i] = 0;
	}
}

void CAT_bag_add(int item_id, int count)
{
	if(item_id >= 0 && item_id < item_table.length)
	{
		item_table.counts[item_id] += count;
	}
}

void CAT_bag_remove(int item_id, int count)
{
	if(item_id >= 0 && item_id < item_table.length)
	{
		count = count == -1 ? item_table.counts[item_id] : count;
		count = max(count, item_table.counts[item_id]);
		item_table.counts[item_id] -= count;
	}
}

static bool basic_filter(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
		return false;
	if(item_table.counts[item_id] <= 0)
		return false;
	return true;
}

static bool prop_filter(int item_id)
{
	return basic_filter(item_id) &&
	item_table.data[item_id].type == CAT_ITEM_TYPE_PROP;
}
static bool tool_filter(int item_id)
{
	return basic_filter(item_id) &&
	item_table.data[item_id].type == CAT_ITEM_TYPE_TOOL;
}
static bool key_filter(int item_id)
{
	return basic_filter(item_id) &&
	item_table.data[item_id].type == CAT_ITEM_TYPE_KEY;
}

struct
{
	const char* title;
	CAT_item_filter filter;
} tabs[] =
{
	{"ALL", basic_filter},
	{"PROPS", prop_filter},
	{"TOOLS", tool_filter},
	{"KEYS", key_filter}
};
#define NUM_TABS (sizeof(tabs)/sizeof(tabs[0]))
static int tab_selector = 0;

int roster_backing[CAT_ITEM_TABLE_CAPACITY];
CAT_int_list roster;

static int base = 0;
static int selector = 0;

void CAT_MS_inventory(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_inventory);
			CAT_ilist(&roster, roster_backing, CAT_ITEM_TABLE_CAPACITY);
			CAT_filter_item_table(tabs[tab_selector].filter, &roster);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			
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
				CAT_filter_item_table(tabs[tab_selector].filter, &roster);
				base = 0;
				selector = 0;
			}
			
			if(roster.length <= 0)
				break;

			if(CAT_input_pulse(CAT_BUTTON_UP))
				selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				selector += 1;
			selector = (selector + roster.length) % roster.length;

			int overshoot = selector - base;
			if(overshoot < 0)
				base += overshoot;
			else if(overshoot >= 9)
				base += (overshoot - 8);

			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_machine_transition(CAT_MS_inspector);
			break;
		}

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_inventory()
{
	CAT_gui_title
	(
		true,
		&icon_enter_sprite, &icon_exit_sprite,
		"%s", tabs[tab_selector].title
	);

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

		int item_id = roster.data[idx];
		CAT_item* item = CAT_item_get(item_id);

		CAT_gui_set_flag(CAT_GUI_FLAG_TIGHT);
		CAT_gui_panel((CAT_ivec2) {0, 2+i*2}, (CAT_ivec2) {15, 2});
		CAT_rowberry(0, (2+i)*32-1, CAT_LCD_SCREEN_W, 0x0000);
		CAT_gui_image(item->icon, 0);
		
		CAT_gui_textf(" %s *%d ", item->name, item_table.counts[item_id]);

		if(idx == selector)
			CAT_gui_image(&icon_pointer_sprite, 0);
	}
}

void CAT_MS_inspector(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_inspector);
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
	int item_id = roster.data[selector];
	CAT_item* item = CAT_item_get(item_id);

	CAT_gui_title
	(
		false,
		NULL, &icon_exit_sprite,
		"INSPECTOR"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	CAT_gui_image(item->sprite, 0);
	CAT_gui_textf(" %s", item->name);
	CAT_gui_div("");

	if(strlen(item->text) > 0)
	{
		CAT_gui_set_flag(CAT_GUI_FLAG_WRAPPED);
		CAT_gui_text(item->text);
		CAT_gui_div("");
	}
}