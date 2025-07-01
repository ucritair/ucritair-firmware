#include "cat_item.h"

#include <stdio.h>
#include "cat_render.h"
#include <string.h>
#include <stddef.h>
#include "cat_input.h"
#include "sprite_assets.h"
#include "cat_gui.h"
#include "cat_curves.h"
#include "item_assets.h"
#include "cat_gizmos.h"

//////////////////////////////////////////////////////////////////////////
// ITEM TABLE

CAT_item* CAT_item_get(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
		return NULL;
	return &item_table.data[item_id];
}

void CAT_filter_item_table(CAT_item_filter filter, CAT_int_list* list)
{
	for(int i = 0; i < item_table.length; i++)
	{
		if(filter == NULL || filter(i))
			CAT_ilist_push(list, i);
	}
}


//////////////////////////////////////////////////////////////////////////
// BAG

void CAT_inventory_clear()
{
	for(int i = 0; i < item_table.length; i++)
	{
		item_table.counts[i] = 0;
	}
}

bool CAT_inventory_add(int item_id, int count)
{
	if(item_id < 0 || item_id >= item_table.length)
		return false;
	uint32_t new_count = item_table.counts[item_id] + count;
	if(new_count > UINT16_MAX)
		return false;
	item_table.counts[item_id] = new_count;
	return true;
}

bool CAT_inventory_remove(int item_id, int count)
{
	if(item_id < 0 || item_id >= item_table.length)
		return false;
	if(count > item_table.counts[item_id])
		return false;
	if(count == -1)
		count = item_table.counts[item_id];
	item_table.counts[item_id] -= count;
	return true;
}

int CAT_inventory_count(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
		return -1;
	return item_table.counts[item_id];
}


//////////////////////////////////////////////////////////////////////////
// INVENTORY

static bool buy_filter(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
		return false;
	return true;
}
static bool prop_filter(int item_id)
{
	return buy_filter(item_id) &&
	item_table.data[item_id].type == CAT_ITEM_TYPE_PROP;
}
static bool tool_filter(int item_id)
{
	return buy_filter(item_id) &&
	item_table.data[item_id].type == CAT_ITEM_TYPE_TOOL;
}
static bool key_filter(int item_id)
{
	return buy_filter(item_id) &&
	item_table.data[item_id].type == CAT_ITEM_TYPE_KEY;
}

static struct
{
	const char* title;
	CAT_item_filter filter;
} item_tabs[] =
{
	{"ALL", buy_filter},
	{"DECORATIONS", prop_filter},
	{"CARE ITEMS", tool_filter},
	{"KEY ITEMS", key_filter}
};
#define NUM_ITEM_TABS (sizeof(item_tabs)/sizeof(item_tabs[0]))
static int inventory_tab_selector = 0;

static int inspect_id = -1;

void CAT_bind_inspector(int item_id)
{
	inspect_id = item_id;
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
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

#define INSPECTOR_BG_COLOUR RGB8882565(142, 171, 174)
#define INSPECTOR_MARGIN 8

void CAT_render_inspector()
{
	CAT_item* item = CAT_item_get(inspect_id);

	CAT_frameberry(INSPECTOR_BG_COLOUR);

	if (item == NULL)
		return;

	int cursor_y = INSPECTOR_MARGIN;
	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_mask(INSPECTOR_MARGIN, -1, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, -1);
	cursor_y = CAT_draw_textf(INSPECTOR_MARGIN, cursor_y, "%s\n", item->name) + 2;
	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf(INSPECTOR_MARGIN, cursor_y, "x%d\n", CAT_inventory_count(inspect_id));
	
	cursor_y += 6;
	CAT_lineberry(INSPECTOR_MARGIN, cursor_y, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, cursor_y, CAT_WHITE);
	cursor_y += 8;

	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_mask(INSPECTOR_MARGIN, -1, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, -1);
	cursor_y = CAT_draw_textf(INSPECTOR_MARGIN, cursor_y, "%s\n", item->text);

	int free_space = CAT_LCD_SCREEN_H-INSPECTOR_MARGIN-cursor_y;
	for(int scale = 1; scale <= 6; scale++)
	{
		if
		(
			item->sprite->width * scale <= 240 &&
			item->sprite->height * scale <= free_space
		)
		{
			CAT_set_sprite_scale(scale);
		}
	}
	cursor_y += free_space / 2;
	
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_draw_sprite(item->sprite, 0, 120, cursor_y);
}

static void inventory_action_proc(int item_id)
{
	CAT_bind_inspector(item_id);
	CAT_machine_transition(CAT_MS_inspector);
}

void CAT_MS_inventory(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_inventory);
			CAT_gui_begin_item_grid_context();
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			
			if(CAT_input_pulse(CAT_BUTTON_LEFT))
				inventory_tab_selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
				inventory_tab_selector += 1;
			inventory_tab_selector = (inventory_tab_selector + NUM_ITEM_TABS) % NUM_ITEM_TABS;
			
			CAT_gui_begin_item_grid(item_tabs[inventory_tab_selector].title, NULL, inventory_action_proc);
			CAT_gui_item_grid_set_flags(CAT_GUI_ITEM_GRID_FLAG_TABS);
			for(int i = 0; i < item_table.length; i++)
			{
				if(item_table.counts[i] <= 0)
					continue;
				if(!item_tabs[inventory_tab_selector].filter(i))
					continue;
				CAT_gui_item_grid_cell(i);
			}
			break;
		}

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_inventory()
{
	// GUI
}

