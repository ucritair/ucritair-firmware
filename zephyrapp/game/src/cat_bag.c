#include "cat_bag.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_actions.h"
#include <string.h>

CAT_bag bag;

void CAT_bag_init()
{
	bag.length = 0;
}

int CAT_bag_find(int item_id)
{
	for(int i = 0; i < bag.length; i++)
	{
		if(bag.item_id[i] == item_id)
		{
			return i;
		}
	}
	return -1;
}

void CAT_bag_add(int item_id)
{
	int idx = CAT_bag_find(item_id);
	if(idx >= 0)
	{
		bag.count[idx] += 1;
	}
	else
	{
		const char* a = item_table.data[item_id].name;
		int insert_idx = 0;
		while(insert_idx < bag.length)
		{
			const char* b = item_table.data[bag.item_id[insert_idx]].name;
			if(strcmp(a, b) < 0)
				break;
			insert_idx += 1;
		}
		for(int i = bag.length; i > insert_idx; i--)
		{
			bag.item_id[i] = bag.item_id[i-1];
			bag.count[i] = bag.count[i-1];
		}
		bag.item_id[insert_idx] = item_id;
		bag.count[insert_idx] = 1;
		bag.length += 1;
	}
}

void CAT_bag_remove(int item_id)
{
	int idx = CAT_bag_find(item_id);
	if(idx >= 0)
	{
		bag.count[idx] -= 1;
		if(bag.count[idx] <= 0)
		{
			for(int i = idx; i < bag.length-1; i++)
			{
				bag.item_id[i] = bag.item_id[i+1];
				bag.count[i] = bag.count[i+1];
			}
			bag.length -= 1;
		}
	}
}

CAT_bag_state bag_state;

void CAT_bag_state_init()
{
	bag_state.base = 0;
	bag_state.idx = 0;
	bag_state.destination = CAT_MS_menu;
}

void CAT_MS_bag(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			bag_state.base = 0;
			bag_state.idx = 0;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, bag_state.destination);
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_room);

			if(bag.length <= 0)
				break;

			if(CAT_input_pulse(CAT_BUTTON_UP))
			{
				bag_state.idx -= 1;
				if(bag_state.idx == -1)
					bag_state.idx = bag.length-1;
			}
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
			{
				bag_state.idx += 1;
				if(bag_state.idx == bag.length)
					bag_state.idx = 0;
			}
			bag_state.idx = clamp(bag_state.idx, 0, bag.length-1);

			int overshoot = bag_state.idx - bag_state.base;
			if(overshoot < 0)
				bag_state.base += overshoot;
			else if(overshoot >= 9)
				bag_state.base += (overshoot - 8);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				int item_id = bag.item_id[bag_state.idx];
				CAT_item* item = &item_table.data[item_id];

				if(item_id == book_item && (bag_state.destination == CAT_MS_menu || bag_state.destination == CAT_MS_study))
				{
					action_state.item_id = item_id;
					CAT_machine_transition(&machine, CAT_MS_study);
				}
				else if(item_id == toy_item && (bag_state.destination == CAT_MS_menu || bag_state.destination == CAT_MS_play))
				{
					action_state.item_id = item_id;
					CAT_machine_transition(&machine, CAT_MS_play);
				}
				else if(item->type == CAT_ITEM_TYPE_FOOD && (bag_state.destination == CAT_MS_menu || bag_state.destination == CAT_MS_feed))
				{
					action_state.item_id = item_id;
					CAT_machine_transition(&machine, CAT_MS_feed);
				}
				else if(item->type == CAT_ITEM_TYPE_PROP && (bag_state.destination == CAT_MS_menu || bag_state.destination == CAT_MS_deco))
				{
					deco_state.add_id = item_id;
					CAT_machine_transition(&machine, CAT_MS_deco);
				}
				else if(item->type == CAT_ITEM_TYPE_GEAR && bag_state.destination == CAT_MS_menu)
				{
					CAT_gear_toggle(item_id, !CAT_gear_status(item_id));
				}
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			bag_state.destination = CAT_MS_menu;
			break;
	}
}

void CAT_render_bag()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("BAG ");
	CAT_gui_image(fbut_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(fbut_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});  
	for(int i = 0; i < 9; i++)
	{
		int idx = bag_state.base + i;
		if(idx >= bag.length)
			return;

		int item_id = bag.item_id[idx];
		CAT_item* item = CAT_item_get(item_id);

		CAT_gui_panel_tight((CAT_ivec2) {0, 2+i*2}, (CAT_ivec2) {15, 2});
		CAT_gui_image(icon_item_sprite, item->type);

		char text[64];
		if(item->type == CAT_ITEM_TYPE_PROP)
			sprintf(text, " %s *%d ", item->name, bag.count[idx]);
		else
			sprintf(text, " %s ", item->name);

		if
		(
			(bag_state.destination == CAT_MS_feed && item->type != CAT_ITEM_TYPE_FOOD) ||
			(bag_state.destination == CAT_MS_study && item_id != book_item) ||
			(bag_state.destination == CAT_MS_play && item_id != toy_item) ||
			(bag_state.destination == CAT_MS_deco && item->type != CAT_ITEM_TYPE_PROP)
		)
		{
			gui.text_mode = CAT_TEXT_MODE_STRIKETHROUGH;
		}	
		CAT_gui_text(text);
		gui.text_mode = CAT_TEXT_MODE_NORMAL;

		if(item->type == CAT_ITEM_TYPE_GEAR)
		{
			int idx = CAT_gear_status(item_id) ? 1 : 0;
			CAT_gui_image(icon_equip_sprite, idx);
			CAT_gui_text(" ");
		}

		if(idx == bag_state.idx)
			CAT_gui_image(icon_pointer_sprite, 0);
	}
}