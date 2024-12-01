#include "cat_arcade.h"

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_sprite.h"
#include "cat_bag.h"
#include <stdio.h>
#include "cat_snake.h"

#ifdef CAT_EMBEDDED
#include "menu_system.h"
#include "menu_aqi.h"
#include "menu_graph.h"
#endif

//////////////////////////////////////////////////////////////////////////
// MACHINE

static int selector = 0;

void CAT_MS_arcade(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

			if(CAT_input_pulse(CAT_BUTTON_UP))
				selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				selector += 1;
			selector = clamp(selector, 0, 2);
			
			if(CAT_input_pressed(CAT_BUTTON_A))
			{
#ifdef CAT_EMBEDDED
				if(selector == 0)
				{
					CAT_machine_transition(CAT_MS_aqi);
				}
				if(selector == 1)
				{
					CAT_machine_transition(CAT_MS_graph);
				}
#endif
				if(selector == 2)
				{
					CAT_machine_transition(CAT_MS_snake);
				}
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_arcade()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("ARCADE ");
	CAT_gui_image(icon_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	// This part is hideous. Ask M about it
	CAT_gui_textf("uCritAir Score %0.1f%%\n", CAT_AQI_aggregate());
	CAT_gui_div("AIR");
	CAT_gui_text("  & Air Quality Log ");
	if(selector == 0)
		CAT_gui_image(icon_pointer_sprite, 0);
	CAT_gui_line_break();
	CAT_gui_text("  & Air Quality Graph ");
	if(selector == 1)
		CAT_gui_image(icon_pointer_sprite, 0);
	CAT_gui_line_break();
	CAT_gui_div("PLAY");
	CAT_gui_text("  & Snack ");
	if(selector == 2)
		CAT_gui_image(icon_pointer_sprite, 0);
}
