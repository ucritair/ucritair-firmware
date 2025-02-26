#include "cat_item_dialog.h"

#include "cat_input.h"
#include "cat_bag.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"

static CAT_item_list roster =
{
	.length = 0,
};
static int base = 0;
static int selector = 0;
CAT_item_filter filter = NULL;
int* anchor = NULL;

void CAT_filter_item_dialog(CAT_item_filter _filter)
{
	filter = _filter;
}

void CAT_anchor_item_dialog(int* _anchor)
{
	anchor = _anchor;
}

void CAT_MS_item_dialog(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_item_list_filter(&bag, &roster, filter);
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
				if(anchor != NULL)
					*anchor = roster.item_ids[selector];
				CAT_machine_back();
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_item_dialog()
{
	CAT_gui_title
	(
		false,
		&icon_enter_sprite, &icon_exit_sprite,
		"SELECT AN ITEM"
	);

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
		
		CAT_gui_set_flag(CAT_GUI_TIGHT);
		CAT_gui_panel((CAT_ivec2) {0, 2+i*2}, (CAT_ivec2) {15, 2});
		CAT_rowberry(0, (2+i)*32-1, LCD_FRAMEBUFFER_W, 0x0000);
		CAT_gui_image(item->icon, 0);
		
		int bag_idx = CAT_item_list_find(&bag, item_id);
		CAT_gui_textf(" %s *%d ", item->name, bag.counts[bag_idx]);

		if(idx == selector)
			CAT_gui_image(&icon_pointer_sprite, 0);
	}
}