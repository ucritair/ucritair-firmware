#include "cat_menu.h"

#include "cat_gui.h"
#include "cat_input.h"

static CAT_button konami_spell[10] =
{
	CAT_BUTTON_UP,
	CAT_BUTTON_UP,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_B,
	CAT_BUTTON_A
};

void CAT_MS_magic(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_input_buffer_clear();
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_held(CAT_BUTTON_B, 0.5f))
				CAT_machine_back();

			if(CAT_input_spell(konami_spell))
				CAT_machine_transition(CAT_MS_hedron);
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_magic()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("MAGIC ");

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18}); 
	CAT_gui_text("Enter an incantation,\n");
	CAT_gui_text("or hold ");
	CAT_gui_image(&icon_b_sprite, 0);
	CAT_gui_text(" to exit.");
	
	CAT_gui_div("INCANTATION");
	int i = (input.buffer_head+9) % 10;
	int steps = 0;
	while(steps < 10)
	{
		if(input.buffer[i] != CAT_BUTTON_LAST)
			CAT_gui_image(&icon_input_sprite, input.buffer[i]);
		i -= 1;
		if(i < 0)
			i = 9;
		steps += 1;
	}
}