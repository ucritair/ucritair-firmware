#include "cat_vending.h"

#include "cat_room.h"
#include "cat_machine.h"
#include "cat_input.h"
#include "cat_gui.h"
#include "cat_item.h"
#include "cat_bag.h"
#include <stdio.h>
#include "cat_sprite.h"

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
			base = 0;
			selector = 0;

			purchase_lock = false;
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
					CAT_item_list_add(&bag, selector);
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
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 4});  
	CAT_gui_text("VENDING MACHINE ");
	CAT_gui_image(icon_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_div("");
	CAT_gui_image(icon_coin_sprite, 0);
	CAT_gui_textf(" $%d", coins);

	CAT_gui_panel((CAT_ivec2) {0, 4}, (CAT_ivec2) {15, 16});  
	for(int i = 0; i < VENDING_MAX_SLOTS; i++)
	{
		int item_id = base + i;
		if(item_id >= item_table.length)
			return;
		
		CAT_item* item = CAT_item_get(item_id);

		CAT_gui_panel_tight((CAT_ivec2) {0, 4+i*2}, (CAT_ivec2) {15, 2});
		CAT_gui_image(icon_item_sprite, item->type);

		CAT_gui_textf(" %s  $%d ", item->name, item->price);

		if(item_id == selector)
		{
			CAT_gui_image(icon_pointer_sprite, 0);
			if(purchase_progress >= 0.01)
			{
				CAT_greenberry(3, 234, 64 + 32 * i, 32, purchase_progress);
			}
			else if(item->price > coins)
			{
				CAT_greyberry(3, 234, 64 + 32 * i, 32);
			}
		}
	}
}