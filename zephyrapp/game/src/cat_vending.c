#include "cat_vending.h"

#include "cat_room.h"
#include "cat_machine.h"
#include "cat_input.h"
#include "cat_gui.h"
#include "cat_item.h"
#include "cat_bag.h"
#include <stdio.h>
#include "cat_render.h"

static float purchase_progress = 0;
static bool purchase_lock = false;

void CAT_MS_vending(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_vending);
			CAT_input_clear();

			purchase_progress = 0;
			purchase_lock = false;
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
					
			CAT_gui_begin_item_list("VENDING MACHINE");
			for(int i = 0; i < item_table.length; i++)
			{
				CAT_item* item = CAT_item_get(i);

				if
				(
					item->type == CAT_ITEM_TYPE_TOOL &&
					(item->data.tool_data.type == CAT_TOOL_TYPE_BOOK ||
					item->data.tool_data.type == CAT_TOOL_TYPE_TOY) &&
					CAT_item_list_find(&bag, i) != -1
				)
				{
					continue;
				}		

				CAT_gui_item_listing(i, 1);
			}
			CAT_gui_item_list_io();
			
			int selection_id = CAT_gui_item_selection();
			CAT_item* selection = CAT_item_get(selection_id);
			if(selection == NULL)
				break;
				
			if(selection->price <= coins && !purchase_lock)
			{
				if(CAT_input_pressed(CAT_BUTTON_A))
					purchase_progress = 0.15f;
				else if(CAT_input_held(CAT_BUTTON_A, 0.0f))
					purchase_progress += CAT_get_delta_time();
				
				if(purchase_progress >= 1)
				{
					CAT_item_list_add(&bag, selection_id, 1);
					coins -= selection->price;
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
	CAT_gui_item_list();

	int selector = CAT_gui_item_selector();
	int item_id = CAT_gui_item_selection();
	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return;

	if(purchase_progress >= 0.01)
	{
		CAT_greenberry(0, 240, 32 + 32 * selector, 32, purchase_progress);
	}
	else if(item->price > coins)
	{
		CAT_greyberry(0, 240, 32 + 32 * selector, 32);
	}
}