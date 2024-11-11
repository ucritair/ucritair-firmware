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

CAT_bag bag;

void CAT_bag_init()
{
	bag.length = 0;
	bag.coins = 5;
}

int CAT_bag_find(int item_id)
{
	for(int i = 0; i < bag.length; i++)
	{
		if(bag.item_ids[i] == item_id)
		{
			return i;
		}
	}
	return -1;
}

void CAT_bag_add(int item_id)
{
	if(bag.length >= CAT_BAG_MAX_LENGTH)
		return;
		
	int idx = CAT_bag_find(item_id);
	if(idx >= 0)
	{
		bag.counts[idx] += 1;
	}
	else
	{
		const char* a = item_table.data[item_id].name;
		int insert_idx = 0;
		while(insert_idx < bag.length)
		{
			const char* b = item_table.data[bag.item_ids[insert_idx]].name;
			if(strcmp(a, b) < 0)
				break;
			insert_idx += 1;
		}
		for(int i = bag.length; i > insert_idx; i--)
		{
			bag.item_ids[i] = bag.item_ids[i-1];
			bag.counts[i] = bag.counts[i-1];
		}
		bag.item_ids[insert_idx] = item_id;
		bag.counts[insert_idx] = 1;
		bag.length += 1;
	}
}

void CAT_bag_remove(int item_id)
{
	int idx = CAT_bag_find(item_id);
	if(idx >= 0)
	{
		bag.counts[idx] -= 1;
		if(bag.counts[idx] <= 0)
		{
			for(int i = idx; i < bag.length-1; i++)
			{
				bag.item_ids[i] = bag.item_ids[i+1];
				bag.counts[i] = bag.counts[i+1];
			}
			bag.length -= 1;
		}
	}
}

struct bag_relation
{
	CAT_machine_state state;
	CAT_item_type type;
	int* ptr;
} bag_relations[] =
{
	{CAT_MS_feed, CAT_ITEM_TYPE_FOOD, &action_state.item_id},
	{CAT_MS_study, CAT_ITEM_TYPE_BOOK, &action_state.item_id},
	{CAT_MS_play, CAT_ITEM_TYPE_TOY, &action_state.item_id},
	{CAT_MS_deco, CAT_ITEM_TYPE_PROP, &deco_state.add_id}
};
#define NUM_BAG_RELATIONS (sizeof(bag_relations)/sizeof(bag_relations[0]))

CAT_machine_state bag_anchor = NULL;
int bag_base = 0;
int bag_selector = 0;

void CAT_MS_bag(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			bag_base = 0;
			bag_selector = 0;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				if(bag_anchor != NULL)
					CAT_machine_transition(&machine, CAT_MS_room);
				else
					CAT_machine_transition(&machine, CAT_MS_menu);
			}			
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_room);

			if(bag.length <= 0)
				break;

			if(CAT_input_pulse(CAT_BUTTON_UP))
			{
				bag_selector -= 1;
				if(bag_selector == -1)
					bag_selector = bag.length-1;				
			}
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
			{
				bag_selector += 1;
				if(bag_selector == bag.length)
					bag_selector = 0;
			}
			bag_selector = clamp(bag_selector, 0, bag.length-1);	

			int overshoot = bag_selector - bag_base;
			if(overshoot < 0)
				bag_base += overshoot;
			else if(overshoot >= 9)
				bag_base += (overshoot - 8);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				int item_id = bag.item_ids[bag_selector];
				CAT_item* item = &item_table.data[item_id];

				for(int i = 0; i < NUM_BAG_RELATIONS; i++)
				{
					struct bag_relation relation = bag_relations[i];
					if((bag_anchor == NULL || relation.state == bag_anchor) && item->type == relation.type)
					{
						*relation.ptr = item_id;
						CAT_machine_transition(&machine, relation.state);
					}
				}

				if(item->type == CAT_ITEM_TYPE_GEAR && bag_anchor == NULL)
				{
					CAT_gear_toggle(item_id, !CAT_gear_status(item_id));
				}
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			bag_anchor = NULL;
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
				"Your bag is currently empty.\n\n"
				"To feed, teach, or entertain\n"
				"your pet, you must purchase\n"
				"food, books, and toys from\n"
				"the vending machine."
			);
			return;
		}

		int idx = bag_base + i;
		if(idx >= bag.length)
			return;

		int item_id = bag.item_ids[idx];
		CAT_item* item = CAT_item_get(item_id);

		CAT_gui_panel_tight((CAT_ivec2) {0, 2+i*2}, (CAT_ivec2) {15, 2});
		CAT_gui_image(icon_item_sprite, item->type);
		
		bool fits_relation = bag_anchor == NULL;
		for(int i = 0; i < NUM_BAG_RELATIONS; i++)
		{
			struct bag_relation relation = bag_relations[i];
			if(relation.state == bag_anchor && relation.type == item->type)
			{
				fits_relation = true;
				break;
			}
		}
		if(!fits_relation)
		{
			gui.text_mode = CAT_TEXT_MODE_STRIKETHROUGH;
		}
		CAT_gui_textf(" %s *%d", item->name, bag.counts[idx]);
		gui.text_mode = CAT_TEXT_MODE_NORMAL;

		if(item->type == CAT_ITEM_TYPE_GEAR && bag_anchor == NULL)
		{
			int idx = CAT_gear_status(item_id) ? 1 : 0;
			CAT_gui_image(icon_equip_sprite, idx);
			CAT_gui_text(" ");
		}

		if(idx == bag_selector)
			CAT_gui_image(icon_pointer_sprite, 0);
	}
}