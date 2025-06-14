#include "cat_item_dialog.h"

#include "cat_input.h"
#include "cat_inventory.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"

CAT_item_filter filter = NULL;
int* target = NULL;
bool enforce_valid_target = false;

void CAT_filter_item_dialog(CAT_item_filter _filter)
{
	filter = _filter;
}

void CAT_target_item_dialog(int* _target, bool enforce_valid)
{
	target = _target;
	enforce_valid_target = enforce_valid;
}

void CAT_MS_item_dialog(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_item_dialog);
			CAT_input_clear();
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				if(enforce_valid_target && *target == -1)
					CAT_machine_transition(CAT_MS_room);
				else
					CAT_machine_back();
			}
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);	

			CAT_gui_set_flag(CAT_GUI_FLAG_INCLUDE_COUNT);
			CAT_gui_begin_item_list("SELECT AN ITEM");
			for(int i = 0; i < item_table.length; i++)
			{
				if(item_table.counts[i] > 0 && filter(i))
				{
					if(CAT_gui_item_listing(i))
					{
						if(target != NULL && (i != -1 || !enforce_valid_target))
						{
							*target = i;
							CAT_machine_back();	
						}
					}
				}
			}
			break;
		}

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_item_dialog()
{
	// Replaced by GUI render pass
}