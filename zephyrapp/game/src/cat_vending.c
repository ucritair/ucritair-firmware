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
			
			CAT_gui_set_flag(CAT_GUI_FLAG_INCLUDE_PRICE);
			CAT_gui_set_flag(CAT_GUI_FLAG_SHOW_COINS);
			CAT_gui_begin_item_list("VENDING MACHINE");
			for(int i = 0; i < item_table.length; i++)
			{
				CAT_item* item = CAT_item_get(i);

				if
				(
					(item->type == CAT_ITEM_TYPE_KEY ||
					(item->type == CAT_ITEM_TYPE_TOOL &&
					(item->data.tool_data.type == CAT_TOOL_TYPE_BOOK ||
					item->data.tool_data.type == CAT_TOOL_TYPE_TOY))) &&
					CAT_item_list_find(&bag, i) != -1
				)
				{
					continue;
				}

				if(item->price == 0)
					continue;

				if(CAT_gui_item_listing(i, 1))
				{
					if(item->price <= coins && !purchase_lock)
					{
						if(CAT_input_pressed(CAT_BUTTON_A))
							purchase_progress = 0.15f;
						else if(CAT_input_held(CAT_BUTTON_A, 0.0f))
							purchase_progress += CAT_get_delta_time_s();
						CAT_gui_item_highlight(purchase_progress);
						
						if(purchase_progress >= 1)
						{
							CAT_item_list_add(&bag, i, 1);
							coins -= item->price;

							purchase_progress = 0;
							purchase_lock = true;
						}
					}
				}
				if(CAT_input_released(CAT_BUTTON_A))
				{
					purchase_progress = 0;
					purchase_lock = false;
				}

				if(item->price > coins)
				{
					CAT_gui_item_greyout();
				}
			}
			
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_vending()
{
	// Replaced by GUI render pass
}