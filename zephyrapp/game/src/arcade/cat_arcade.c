#include "cat_arcade.h"

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_render.h"
#include "cat_item.h"
#include <stdio.h>
#include "cat_aqi.h"
#include "sprite_assets.h"

//////////////////////////////////////////////////////////////////////////
// MACHINE

void CAT_MS_arcade(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_arcade);
			CAT_gui_begin_menu_context();
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_gui_begin_menu("ARCADE"))
			{
				if(CAT_gui_menu_item("SNACK"))
					CAT_machine_transition(CAT_MS_snake);
				if(CAT_gui_menu_item("MINES"))
					CAT_machine_transition(CAT_MS_mines);
				if(CAT_gui_menu_item("FOURSQUARES"))
					CAT_machine_transition(CAT_MS_foursquares);
				CAT_gui_end_menu();
			}
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_arcade()
{
	// Replaced by GUI render pass
}
