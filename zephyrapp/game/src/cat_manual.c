#include "cat_manual.h"

#include "cat_machine.h"
#include "cat_sprite.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_bag.h"

void CAT_MS_manual(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(CAT_MS_menu);
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_manual()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("CONTROLS ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	CAT_gui_image(icon_start_sprite, 0);
	CAT_gui_text("Open/close menu");
	CAT_gui_line_break();
	CAT_gui_image(icon_select_sprite, 0);
	CAT_gui_text("Cycle decor mode");
	CAT_gui_line_break();
	CAT_gui_image(icon_a_sprite, 0);
	CAT_gui_text("Confirm");
	CAT_gui_line_break();
	CAT_gui_image(icon_b_sprite, 0);
	CAT_gui_text("Cancel");
	CAT_gui_line_break();
	CAT_gui_image(icon_n_sprite, 0);
	CAT_gui_text("Navigate up");
	CAT_gui_line_break();
	CAT_gui_image(icon_e_sprite, 0);
	CAT_gui_text("Navigate right");
	CAT_gui_line_break();
	CAT_gui_image(icon_s_sprite, 0);
	CAT_gui_text("Navigate down");
	CAT_gui_line_break();
	CAT_gui_image(icon_w_sprite, 0);
	CAT_gui_text("Navigate left");
}