#include "cat_menu.h"

#include "cat_gui.h"
#include "cat_input.h"
#include "cat_arcade.h"
#include "sprite_assets.h"
#include "cat_main.h"

CAT_button basic_spell[10] =
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

void CAT_MS_magic(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_magic);
			CAT_input_buffer_clear();
			break;
		case CAT_FSM_SIGNAL_TICK:
			if(CAT_input_held(CAT_BUTTON_B, 0.5f))
				CAT_pushdown_pop();

			if(CAT_input_spell(basic_spell))
			{
				CAT_raise_config_flags(CAT_CONFIG_FLAG_DEVELOPER);
				CAT_pushdown_push(CAT_MS_hedron);
			}		
			break;
		case CAT_FSM_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_magic()
{
	CAT_gui_title(false, "MAGIC");

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18}); 
	CAT_gui_text("Enter an incantation,\n");
	CAT_gui_text("or hold [B] to exit");
	
	CAT_gui_div("INCANTATION");
	int i = (CAT_input_buffer_head()+9) % 10;
	int steps = 0;
	while(steps < 10)
	{
		if(CAT_input_buffer_get(i) != CAT_BUTTON_LAST)
			CAT_gui_image(&icon_input_sprite, CAT_input_buffer_get(i));
		i -= 1;
		if(i < 0)
			i = 9;
		steps += 1;
	}
}