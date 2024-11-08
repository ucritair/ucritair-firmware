#include "menu_time.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_actions.h"
#include "cat_bag.h"

void CAT_MS_time(CAT_machine_signal signal)
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
				CAT_machine_transition(&machine, CAT_MS_menu);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			bag_state.destination = CAT_MS_menu;
			break;
	}
}

void CAT_render_time()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("SET TIME ");
	CAT_gui_image(fbut_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_line_break();
	CAT_gui_text("some stuff");

	// CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});  
	// for(int i = 0; i < 9; i++)
	// {
	// 	int idx = bag_state.base + i;
	// 	if(idx >= bag.length)
	// 		return;

	// 	int item_id = bag.item_id[idx];
	// 	CAT_item* item = CAT_item_get(item_id);

	// 	CAT_gui_panel_tight((CAT_ivec2) {0, 2+i*2}, (CAT_ivec2) {15, 2});
	// 	CAT_gui_image(icon_item_sprite, item->type);

	// 	char text[64];
	// 	if(item->type == CAT_ITEM_TYPE_PROP)
	// 		sprintf(text, " %s *%d ", item->name, bag.count[idx]);
	// 	else
	// 		sprintf(text, " %s ", item->name);

	// 	if
	// 	(
	// 		(bag_state.destination == CAT_MS_feed && item->type != CAT_ITEM_TYPE_FOOD) ||
	// 		(bag_state.destination == CAT_MS_study && item_id != book_item) ||
	// 		(bag_state.destination == CAT_MS_play && item_id != toy_item) ||
	// 		(bag_state.destination == CAT_MS_deco && item->type != CAT_ITEM_TYPE_PROP)
	// 	)
	// 	{
	// 		gui.text_mode = CAT_TEXT_MODE_STRIKETHROUGH;
	// 	}	
	// 	CAT_gui_text(text);
	// 	gui.text_mode = CAT_TEXT_MODE_NORMAL;

	// 	if(item->type == CAT_ITEM_TYPE_GEAR)
	// 	{
	// 		int idx = CAT_gear_status(item_id) ? 1 : 0;
	// 		CAT_gui_image(icon_equip_sprite, idx);
	// 		CAT_gui_text(" ");
	// 	}

	// 	if(idx == bag_state.idx)
	// 		CAT_gui_image(icon_pointer_sprite, 0);
	// }
}
