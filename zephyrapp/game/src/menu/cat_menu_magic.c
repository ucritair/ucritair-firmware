#include "cat_menu.h"

#include "cat_gui.h"
#include "cat_input.h"
#include "cat_arcade.h"
#include "sprite_assets.h"
#include "cat_main.h"
#include "cat_save.h"

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
				CAT_raise_config_flags(CAT_SAVE_CONFIG_FLAG_DEVELOPER);
				CAT_pushdown_push(CAT_MS_hedron);
			}		
			break;
		case CAT_FSM_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_magic()
{
	CAT_frameberry(CAT_WHITE);

	int cursor_x = 12;
	int cursor_y = 12;

	cursor_y = CAT_draw_text(cursor_x, cursor_y, "MAGIC\n");
	cursor_y += 6;
	CAT_rowberry(cursor_y, cursor_y+1, CAT_BLACK);
	cursor_y += 6;

	CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	cursor_y = CAT_draw_text(cursor_x, cursor_y, "Enter an incantation, or hold [B] to exit.\n");
	cursor_y += 6;
	CAT_rowberry(cursor_y, cursor_y+1, CAT_BLACK);
	cursor_y += 6;
	
	int i = (CAT_input_buffer_head()+9) % 10;
	int steps = 0;
	while(steps < 10)
	{
		if(CAT_input_buffer_get(i) != CAT_BUTTON_LAST)
		{
			CAT_draw_sprite_raw(&icon_input_sprite, CAT_input_buffer_get(i), cursor_x, cursor_y);
			cursor_x += icon_input_sprite.width + 6;
		}

		i -= 1;
		if(i < 0)
			i = 9;
		steps += 1;
	}
}