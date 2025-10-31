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

int snake_highscore = 0;
int mines_highscore = 0;
int foursquares_highscore = 0;
int stroop_highscore = 0;

//////////////////////////////////////////////////////////////////////////
// MACHINE

void CAT_MS_arcade(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_arcade);
			CAT_gui_begin_menu_context();
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_gui_begin_menu("ARCADE"))
			{
				if(CAT_gui_menu_item("SNACK"))
					CAT_pushdown_push(CAT_MS_snake);
				if(CAT_gui_menu_item("SWEEP"))
					CAT_pushdown_push(CAT_MS_mines);
				if(CAT_gui_menu_item("FOURSQUARES"))
					CAT_pushdown_push(CAT_MS_foursquares);
				if(CAT_gui_menu_item("STROOP"))
					CAT_pushdown_push(CAT_MS_stroop);
				CAT_gui_end_menu();
			}
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_arcade()
{
	// Replaced by GUI render pass
}
