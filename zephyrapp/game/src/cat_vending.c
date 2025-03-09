#include "cat_vending.h"

#include "cat_room.h"
#include "cat_machine.h"
#include "cat_input.h"
#include "cat_gui.h"
#include "cat_item.h"
#include "cat_bag.h"
#include <stdio.h>
#include "cat_render.h"

#define VENDING_MAX_SLOTS 8

static int base = 0;
static int selector = 0;

float purchase_progress = 0;
bool purchase_lock = false;

void CAT_MS_vending(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_vending);
			base = 0;
			selector = 0;
			purchase_lock = false;
			CAT_input_clear();
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
					
			if(CAT_input_pulse(CAT_BUTTON_UP))
			{
				selector -= 1;
				if(selector == -1)
					selector = item_table.length-1;
				purchase_progress = 0;
			}
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
			{
				selector += 1;
				if(selector == item_table.length)
					selector = 0;
				purchase_progress = 0;
			}
			selector = clamp(selector, 0, item_table.length-1);

			int overshoot = selector - base;
			if(overshoot < 0)
				base += overshoot;
			else if(overshoot >= VENDING_MAX_SLOTS)
				base += (overshoot - VENDING_MAX_SLOTS + 1);

			CAT_item* item = CAT_item_get(selector);
			if(item->price <= coins && !purchase_lock)
			{
				if(CAT_input_pressed(CAT_BUTTON_A))
					purchase_progress = 0.15f;
				else if(CAT_input_held(CAT_BUTTON_A, 0.0f))
					purchase_progress += CAT_get_delta_time();
				
				if(purchase_progress >= 1)
				{
					CAT_item_list_add(&bag, selector, 1);
					coins -= item->price;
					purchase_progress = 0;
					purchase_lock = true;
				}
			}
			if(CAT_input_released(CAT_BUTTON_A))
			{
				purchase_progress = 0;
				purchase_lock = false;
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_vending()
{
	CAT_gui_title
	(
		false,
		&icon_enter_sprite, &icon_exit_sprite,
		"VENDING MACHINE"
	);

	CAT_gui_set_flag(CAT_GUI_TIGHT);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 2});
	CAT_gui_image(&icon_coin_sprite, 0);
	CAT_gui_textf(" $%d", coins);
	CAT_rowberry(0, 16 * 4 - 1, LCD_FRAMEBUFFER_W, 0x0000);

	CAT_gui_panel((CAT_ivec2) {0, 4}, (CAT_ivec2) {15, 16});
	int item_idx = 0;
	int slot_idx = 0;
	while(slot_idx < VENDING_MAX_SLOTS)
	{
		int item_id = base + item_idx;
		if(item_id >= item_table.length)
			return;
		CAT_item* item = CAT_item_get(item_id);
		item_idx += 1;
		
		if
		(
			item->type == CAT_ITEM_TYPE_TOOL &&
			(item->data.tool_data.type == CAT_TOOL_TYPE_BOOK ||
			item->data.tool_data.type == CAT_TOOL_TYPE_TOY) &&
			CAT_item_list_find(&bag, item_id) != -1
		)
		{
			continue;
		}

		CAT_gui_set_flag(CAT_GUI_TIGHT);
		CAT_gui_panel((CAT_ivec2) {0, 4+slot_idx*2}, (CAT_ivec2) {15, 2});
		CAT_rowberry(0, (2+slot_idx)*32-1, LCD_FRAMEBUFFER_W, 0x0000);
		CAT_gui_image(item->icon, 0);

		CAT_gui_textf(" %s  $%d", item->name, item->price);

		if(item_idx == selector)
		{
			CAT_gui_image(&icon_pointer_sprite, 0);
			if(purchase_progress >= 0.01)
			{
				CAT_greenberry(0, 240, 64 + 32 * slot_idx, 32, purchase_progress);
			}
			else if(item->price > coins)
			{
				CAT_greyberry(0, 240, 64 + 32 * slot_idx, 32);
			}
		}

		slot_idx += 1;
	}
}