#include "cat_item.h"

#include <stdio.h>
#include "cat_render.h"
#include <string.h>
#include <stddef.h>
#include "cat_input.h"
#include "sprite_assets.h"
#include "cat_gui.h"
#include "cat_curves.h"

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

int coins = 0;

void CAT_inventory_clear()
{
	for(int i = 0; i < CAT_ITEM_TABLE_CAPACITY; i++)
	{
		item_table.counts[i] = 0;
	}
}

void CAT_inventory_add(int item_id, int count)
{
	if(item_id >= 0 && item_id < item_table.length)
	{
		item_table.counts[item_id] += count;
	}
}

void CAT_inventory_remove(int item_id, int count)
{
	if(item_id >= 0 && item_id < item_table.length)
	{
		count = count == -1 ? item_table.counts[item_id] : count;
		count = max(count, item_table.counts[item_id]);
		item_table.counts[item_id] -= count;
	}
}


//////////////////////////////////////////////////////////////////////////
// INVENTORY

static bool basic_filter(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
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

static struct
{
	const char* title;
	CAT_item_filter filter;
} item_tabs[] =
{
	{"ALL", basic_filter},
	{"PROPS", prop_filter},
	{"TOOLS", tool_filter},
	{"KEYS", key_filter}
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
	cursor_y = CAT_draw_textf(INSPECTOR_MARGIN, cursor_y, "x%d\n", item->price, item_table.counts[inspect_id]);
	
	cursor_y += 6;
	CAT_lineberry(INSPECTOR_MARGIN, cursor_y, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, cursor_y, CAT_WHITE);
	cursor_y += 8;

	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_mask(INSPECTOR_MARGIN, -1, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, -1);
	cursor_y = CAT_draw_text(INSPECTOR_MARGIN, cursor_y, item->text);

	float aspect = item->sprite->height / (float) item->sprite->width;
	int draw_y = 200;
	if(aspect <= 1.25f)
		CAT_set_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	else
	{
		draw_y = 300;
		CAT_set_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_BOTTOM);
	}
	for(int scale = 1; scale <= 6; scale++)
	{
		if
		(
			item->sprite->width * scale <= 240 &&
			item->sprite->height * scale <= 160
		)
			CAT_set_draw_scale(scale);
	}
	CAT_draw_sprite(item->sprite, 0, 120, draw_y);
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


//////////////////////////////////////////////////////////////////////////
// SHOP

static int checkout_id = -1;
static int purchase_qty = 1;

static float purchase_progress = 0;
static bool purchase_lock = false;

void CAT_bind_checkout(int item_id)
{
	checkout_id = item_id;
	purchase_qty = 1;

	purchase_progress = 0;
	purchase_lock = false;
}

void CAT_MS_checkout(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_checkout);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();

			if(CAT_input_pulse(CAT_BUTTON_LEFT))
				purchase_qty -= 1;
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
				purchase_qty += 1;
			
			int purchasable = coins / item_table.data[checkout_id].price;
			if(purchasable <= 0)
				purchase_qty = 0;
			else
			{
				int max_qty = min(purchasable, 99);
				max_qty = min(max_qty, UINT16_MAX-item_table.counts[checkout_id]);
				purchase_qty = clamp(purchase_qty, 1, max_qty);
			}

			if(CAT_input_held(CAT_BUTTON_A, 0) && purchase_qty > 0)
			{
				if(CAT_input_pressed(CAT_BUTTON_A))
					purchase_progress += 0.1f;

				purchase_progress += CAT_get_delta_time_s();	
				purchase_progress = clamp(purchase_progress, 0, 1);

				if(!purchase_lock)
				{	
					if(purchase_progress >= 0.9f)
						purchase_lock = true;
				}
			}
			else if(CAT_input_released(CAT_BUTTON_A))
			{
				if(purchase_lock)
				{
					item_table.counts[checkout_id] += purchase_qty;
					coins -= item_table.data[checkout_id].price * purchase_qty;
				}

				purchase_progress = 0;
				purchase_lock = false;	
			}

			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

#define CHECKOUT_GOLD_COLOUR 0xf606

void CAT_render_checkout()
{
	CAT_item* item = CAT_item_get(checkout_id);

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
	cursor_y = CAT_draw_textf(INSPECTOR_MARGIN, cursor_y, "$%d / x%d owned", item->price, item_table.counts[checkout_id]);
	CAT_set_text_colour(CHECKOUT_GOLD_COLOUR);
	cursor_y = CAT_draw_textf(CAT_LCD_SCREEN_W-INSPECTOR_MARGIN-CAT_GLYPH_WIDTH*7, cursor_y, "$%.6d\n", coins);
	
	cursor_y += 6;
	CAT_lineberry(INSPECTOR_MARGIN, cursor_y, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, cursor_y, CAT_WHITE);
	cursor_y += 12;

	CAT_set_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	for(int scale = 1; scale <= 6; scale++)
	{
		if
		(
			item->sprite->width * scale <= 240 &&
			item->sprite->height * scale <= 160
		)
			CAT_set_draw_scale(scale);
	}
	CAT_set_draw_mask(INSPECTOR_MARGIN, cursor_y, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, cursor_y + 160);
	CAT_draw_sprite(item->sprite, 0, 120, cursor_y + 80);
	CAT_strokeberry(INSPECTOR_MARGIN, cursor_y, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN * 2, 160, CAT_WHITE);
	cursor_y += 160;

	cursor_y += 32;

	if(purchase_qty > 0)
	{
		uint16_t colour = purchase_lock ? CHECKOUT_GOLD_COLOUR : CAT_WHITE;

		CAT_set_text_scale(2);
		CAT_set_text_colour(colour);
		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
		cursor_y = CAT_draw_textf(120, cursor_y, "%d", purchase_qty);

		int size = purchase_lock ? 36 : 32;
		float progress = CAT_ease_out_quart(purchase_progress);
		int anchor_l = 120 - size - progress * 32;
		int anchor_r = 120 + size + progress * 32;
		int anchor_y = cursor_y + 12;
		CAT_lineberry(anchor_l, anchor_y, anchor_l + size, anchor_y + size, colour);
		CAT_lineberry(anchor_l, anchor_y, anchor_l + size, anchor_y - size, colour);
		CAT_lineberry(anchor_r, anchor_y, anchor_r - size, anchor_y + size, colour);
		CAT_lineberry(anchor_r, anchor_y, anchor_r - size, anchor_y - size, colour);
	}
	else
	{
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER | CAT_TEXT_FLAG_WRAP);
		cursor_y = CAT_draw_text(120, cursor_y-20, "Insufficient funds!");
	}
}

static int shop_tab_selector = 0;

static void shop_action_proc(int item_id)
{
	CAT_bind_checkout(item_id);
	CAT_machine_transition(CAT_MS_checkout);
}

void CAT_MS_shop(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_shop);
			CAT_gui_begin_item_grid_context();
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			
			if(CAT_input_pulse(CAT_BUTTON_LEFT))
				shop_tab_selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
				shop_tab_selector += 1;
			shop_tab_selector = (shop_tab_selector + NUM_ITEM_TABS) % NUM_ITEM_TABS;
			
			CAT_gui_begin_item_grid(item_tabs[shop_tab_selector].title, NULL, shop_action_proc);
			for(int i = 0; i < item_table.length; i++)
			{
				if(item_table.data[i].price <= 0)
					continue;
				if(!item_tabs[shop_tab_selector].filter(i))
					continue;
				if
				(
					item_table.data[i].type == CAT_ITEM_TYPE_TOOL &&
					item_table.data[i].data.tool_data.type != CAT_TOOL_TYPE_FOOD &&
					item_table.counts[i] > 0
				)
					continue;

				CAT_gui_item_grid_cell(i);
			}
			break;
		}

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_shop()
{
	// GUI
}