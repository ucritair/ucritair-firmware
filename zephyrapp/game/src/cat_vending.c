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

CAT_vending_state vending_state;

void CAT_MS_vending(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			vending_state.base = 0;
			vending_state.idx = 0;
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pulse(CAT_BUTTON_UP))
			{
				vending_state.idx -= 1;
				if(vending_state.idx == -1)
					vending_state.idx = item_table.length-1;
			}
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
			{
				vending_state.idx += 1;
				if(vending_state.idx == item_table.length)
					vending_state.idx = 0;
			}
			vending_state.idx = clamp(vending_state.idx, 0, item_table.length-1);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				int price = item_table.data[vending_state.idx].price;
				if(bag.coins >= price)
				{
					CAT_bag_add(vending_state.idx);
					bag.coins -= price;
				}
			}

			int overshoot = vending_state.idx - vending_state.base;
			if(overshoot < 0)
				vending_state.base += overshoot;
			else if(overshoot >= VENDING_MAX_SLOTS)
				vending_state.base += (overshoot - VENDING_MAX_SLOTS + 1);

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
	char text[64];
	sprintf(text, " $%d", bag.coins);
	CAT_gui_text(text);

	CAT_gui_panel((CAT_ivec2) {0, 4}, (CAT_ivec2) {15, 16});  
	for(int i = 0; i < VENDING_MAX_SLOTS; i++)
	{
		int item_id = vending_state.base + i;
		if(item_id >= item_table.length)
			return;
		
		CAT_item* item = CAT_item_get(item_id);

		CAT_gui_panel_tight((CAT_ivec2) {0, 4+i*2}, (CAT_ivec2) {15, 2});
		CAT_gui_image(icon_item_sprite, item->type);

		sprintf(text, " %s  $%d", item->name, item->price);
		CAT_gui_text(text);
		gui.text_mode = CAT_TEXT_MODE_NORMAL;

		if(item_id == vending_state.idx)
			CAT_gui_image(icon_pointer_sprite, 0);
	}
}