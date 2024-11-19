#include "cat_item_dialog.h"

#include "cat_input.h"
#include "cat_bag.h"
#include "cat_sprite.h"
#include "cat_gui.h"
#include "cat_room.h"

static struct CAT_bag_anchor
{
	CAT_machine_state state;
	CAT_item_type type;
	int* ptr;
} anchor =
{
	.state = NULL,
	.type = CAT_ITEM_TYPE_KEY,
	.ptr = NULL
};

static CAT_item_list roster =
{
	.length = 0,
};
static int base = 0;
static int selector = 0;

void CAT_anchor_item_dialog(CAT_machine_state state, CAT_item_type type, int* ptr)
{
	anchor.state = state;
	anchor.type = type;
	anchor.ptr = ptr;
}

void CAT_MS_item_dialog(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			roster.length = 0;
			for(int i = 0; i < bag.length; i++)
			{
				int item_id = bag.item_ids[i];
				CAT_item* item = CAT_item_get(item_id);
				if(item->type == anchor.type && bag.counts[i] > 0)
				{
					CAT_item_list_add(&roster, item_id);
				}
			}

			base = 0;
			selector = 0;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);	

			if(roster.length <= 0)
				break;

			if(CAT_input_pulse(CAT_BUTTON_UP))
			{
				selector -= 1;
				if(selector == -1)
					selector = roster.length-1;				
			}
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
			{
				selector += 1;
				if(selector == roster.length)
					selector = 0;
			}
			selector = clamp(selector, 0, roster.length-1);	

			int overshoot = selector - base;
			if(overshoot < 0)
				base += overshoot;
			else if(overshoot >= 9)
				base += (overshoot - 8);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				int item_id = roster.item_ids[selector];
				*anchor.ptr = item_id;
				CAT_machine_transition(anchor.state);
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			CAT_anchor_item_dialog(NULL, CAT_ITEM_TYPE_KEY, NULL);
			break;
	}
}

void CAT_render_item_dialog()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("SELECT AN ITEM ");
	CAT_gui_image(icon_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});  
	for(int i = 0; i < 9; i++)
	{
		if(roster.length == 0)
		{
			CAT_gui_text
			(
				"You do not have any\n"
				"appropriate items."
			);
			return;
		}

		int idx = base + i;
		if(idx >= roster.length)
			return;

		int item_id = roster.item_ids[idx];
		CAT_item* item = CAT_item_get(item_id);

		CAT_gui_panel_tight((CAT_ivec2) {0, 2+i*2}, (CAT_ivec2) {15, 2});
		CAT_gui_image(icon_item_sprite, item->type);
		
		CAT_gui_textf(" %s *%d", item->name, bag.counts[idx]);

		if(idx == selector)
			CAT_gui_image(icon_pointer_sprite, 0);
	}
}