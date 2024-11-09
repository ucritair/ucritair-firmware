#include "menu_time.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_actions.h"
#include "cat_bag.h"
#include "rtc.h"

void CAT_MS_aqi(CAT_machine_signal signal)
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

void CAT_render_aqi()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("AIR QUALITY ");
	CAT_gui_image(fbut_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_line_break();

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	
	char buf[64];
#define textf(...) snprintf(buf, sizeof(buf), __VA_ARGS__); CAT_gui_text(buf)
	
	textf("ABC");
}
