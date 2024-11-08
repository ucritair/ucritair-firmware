#include "cat_menu.h"
#include "cat_room.h"
#include "cat_gui.h"
#include "cat_input.h"

CAT_menu_state menu_state;

void CAT_menu_state_init()
{
	menu_state.titles[0] = "INSIGHTS";
	menu_state.states[0] = CAT_MS_stats;
	menu_state.titles[1] = "BAG";
	menu_state.states[1] = CAT_MS_bag;
	menu_state.titles[2] = "ARCADE CABINET";
	menu_state.states[2] = CAT_MS_arcade;
	menu_state.titles[3] = "VENDING MACHINE";
	menu_state.states[3] = CAT_MS_vending;
	menu_state.titles[4] = "CONTROLS";
	menu_state.states[4] = CAT_MS_manual;
	menu_state.titles[5] = "BACK";
	menu_state.states[5] = CAT_MS_room;
	menu_state.selector = 0;
}

void CAT_MS_menu(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pulse(CAT_BUTTON_UP))
				menu_state.selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				menu_state.selector += 1;
			menu_state.selector = clamp(menu_state.selector, 0, 5);

			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_machine_transition(&machine, menu_state.states[menu_state.selector]);

			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_room);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_menu()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("MENU ");
	CAT_gui_image(fbut_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(fbut_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});  
	for(int i = 0; i < 6; i++)
	{
		CAT_gui_text("#");
		CAT_gui_text(menu_state.titles[i]);

		if(i == menu_state.selector)
			CAT_gui_image(icon_pointer_sprite, 0);

		CAT_gui_line_break();
	}
}