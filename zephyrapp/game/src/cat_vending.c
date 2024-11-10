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

int vending_base = 0;
int vending_selector = 0;
bool purchase_lock = false;
float purchase_progress = 0;

void CAT_MS_vending(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			vending_base = 0;
			vending_selector = 0;
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pulse(CAT_BUTTON_UP))
			{
				vending_selector -= 1;
				if(vending_selector == -1)
					vending_selector = item_table.length-1;
				CAT_input_clear(CAT_BUTTON_A);
			}
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
			{
				vending_selector += 1;
				if(vending_selector == item_table.length)
					vending_selector = 0;
				CAT_input_clear(CAT_BUTTON_A);
			}
			vending_selector = clamp(vending_selector, 0, item_table.length-1);

			if(!purchase_lock)
			{
				purchase_progress = CAT_input_progress(CAT_BUTTON_A, 1.5f);
				if(purchase_progress >= 1)
				{
					int price = item_table.data[vending_selector].price;
					if(bag.coins >= price)
					{
						CAT_bag_add(vending_selector);
						bag.coins -= price;
						purchase_lock = true;
						purchase_progress = 0;
					}
				}
			}
			if(CAT_input_released(CAT_BUTTON_A))
				purchase_lock = false;

			int overshoot = vending_selector - vending_base;
			if(overshoot < 0)
				vending_base += overshoot;
			else if(overshoot >= VENDING_MAX_SLOTS)
				vending_base += (overshoot - VENDING_MAX_SLOTS + 1);

			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_menu);
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_room);
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
	CAT_gui_textf(" $%d", bag.coins);

	CAT_gui_panel((CAT_ivec2) {0, 4}, (CAT_ivec2) {15, 16});  
	for(int i = 0; i < VENDING_MAX_SLOTS; i++)
	{
		int item_id = vending_base + i;
		if(item_id >= item_table.length)
			return;
		
		CAT_item* item = CAT_item_get(item_id);

		CAT_gui_panel_tight((CAT_ivec2) {0, 4+i*2}, (CAT_ivec2) {15, 2});
		CAT_gui_image(icon_item_sprite, item->type);

		CAT_gui_textf(" %s  $%d", item->name, item->price);

		if(item_id == vending_selector)
			CAT_gui_image(icon_pointer_sprite, 0);
	}

	CAT_greenberry(3, 15*16-6, 4 * 16 + 32 * (vending_selector-vending_base) + 3, 32-6, purchase_progress);
}