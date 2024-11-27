#include "cat_bag.h"

#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_actions.h"
#include <string.h>
#include "cat_sprite.h"
#include <stddef.h>
#include "cat_menu.h"

CAT_item_list bag =
{
	.length = 0,
};
int coins = 0;

static int base = 0;
static int selector = 0;

void CAT_MS_bag(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
					CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

			if(bag.length <= 0)
				break;

			if(CAT_input_pulse(CAT_BUTTON_UP))
			{
				selector -= 1;
				if(selector == -1)
					selector = bag.length-1;				
			}
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
			{
				selector += 1;
				if(selector == bag.length)
					selector = 0;
			}
			selector = clamp(selector, 0, bag.length-1);

			int overshoot = selector - base;
			if(overshoot < 0)
				base += overshoot;
			else if(overshoot >= 9)
				base += (overshoot - 8);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				CAT_machine_transition(CAT_MS_inspector);
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_bag()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("BAG ");
	CAT_gui_image(icon_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});  
	for(int i = 0; i < 9; i++)
	{
		if(bag.length == 0)
		{
			CAT_gui_text
			(
				"Your bag is currently empty.\n"
				"\n"
				"Items can be purchased from\n"
				"the vending machine"
			);
			return;
		}

		int idx = base + i;
		if(idx >= bag.length)
			return;

		int item_id = bag.item_ids[idx];
		CAT_item* item = CAT_item_get(item_id);

		CAT_gui_panel_tight((CAT_ivec2) {0, 2+i*2}, (CAT_ivec2) {15, 2});
		CAT_gui_image(icon_item_sprite, item->type);
		
		CAT_gui_textf(" %s *%d ", item->name, bag.counts[idx]);

		if(item->type == CAT_ITEM_TYPE_GEAR)
		{
			int idx = CAT_gear_status(item_id) ? 1 : 0;
			CAT_gui_image(icon_equip_sprite, idx);
			CAT_gui_text(" ");
		}

		if(idx == selector)
			CAT_gui_image(icon_pointer_sprite, 0);
	}
}

void CAT_MS_inspector(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			
			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				int item_id = bag.item_ids[selector];
				CAT_item* item = CAT_item_get(item_id);
				if(item->type == CAT_ITEM_TYPE_GEAR)
				{
					CAT_gear_toggle(item_id, !CAT_gear_status(item_id));
				}
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_render_inspector()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("ITEM INSPECTOR ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	int item_id = bag.item_ids[selector];
	CAT_item* item = CAT_item_get(item_id);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	CAT_gui_image(item->sprite_id, 0);
	CAT_gui_text(item->name);
	CAT_gui_div("");

	if(strlen(item->text) > 0)
	{
		CAT_gui_text(item->text);
		CAT_gui_div("");
	}

	if(item->type == CAT_ITEM_TYPE_GEAR)
	{
		CAT_gui_text("EQUIP: ");
		int idx = CAT_gear_status(item_id) ? 1 : 0;
		CAT_gui_image(icon_equip_sprite, idx);
		CAT_gui_text(" ");
		CAT_gui_image(icon_pointer_sprite, 0);
	}
}