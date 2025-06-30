#include "cat_menu.h"

#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_pet.h"
#include "cat_render.h"
#include <string.h>
#include "config.h"
#include "cat_aqi.h"
#include "sprite_assets.h"
#include "item_assets.h"
#include "cat_gizmos.h"

void CAT_MS_insights(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_insights);
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

#define MARGIN 12

int draw_stat_pips(int cursor_y, const char* name, uint16_t colour, int value)
{
	int cursor_x = MARGIN;
	int text_width = (strlen(name) + 1) * CAT_GLYPH_WIDTH;
	CAT_set_text_colour(colour);
	CAT_draw_textf(MARGIN, cursor_y, name);
	cursor_x += text_width;

	for(int i = 0; i < 12; i++)
	{
		if(i < value)
			CAT_fillberry(cursor_x, cursor_y-1, CAT_GLYPH_WIDTH, CAT_GLYPH_HEIGHT+2, colour);
		else
			CAT_strokeberry(cursor_x, cursor_y-1, CAT_GLYPH_WIDTH, CAT_GLYPH_HEIGHT+2, CAT_WHITE);
		cursor_x += CAT_GLYPH_WIDTH + 4;
	}

	return cursor_y + CAT_GLYPH_HEIGHT + 4;
}

void CAT_render_insights()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = 12;

	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "%s\n", pet.name);
	CAT_lineberry(MARGIN, cursor_y, CAT_LCD_SCREEN_W-MARGIN, cursor_y, CAT_WHITE);
	cursor_y += 8;

	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "LEVEL %d \1 XP %d/%d\n", pet.level+1, pet.xp, level_cutoffs[pet.level]);

	CAT_set_sprite_scale(2);
	CAT_draw_sprite(AS_idle.tick_sprite, 0, MARGIN, cursor_y);
	cursor_y += (AS_idle.tick_sprite->height*2) + 4;

	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "LIFETIME %d \1 LIFESPAN %d\n", pet.lifetime, pet.lifespan);

	cursor_y += 8;

	cursor_y = draw_stat_pips(cursor_y, "VIG", CAT_VIGOUR_ORANGE, pet.vigour);
	cursor_y = draw_stat_pips(cursor_y, "FOC", CAT_FOCUS_BLUE, pet.focus);
	cursor_y = draw_stat_pips(cursor_y, "SPI", CAT_SPIRIT_PURPLE, pet.spirit);

	cursor_y += 8;

	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "ACTIVE DEFENSES\n");
	CAT_lineberry(MARGIN, cursor_y, CAT_LCD_SCREEN_W-MARGIN, cursor_y, CAT_WHITE);
	cursor_y += 4;

	int cursor_x = MARGIN;
	if(CAT_inventory_count(mask_item) > 0)
	{
		CAT_draw_sprite(&icon_mask_sprite, 0, cursor_x, cursor_y);
		cursor_x += icon_mask_sprite.width + 4;
	}
	if(CAT_room_find(prop_uv_lamp_item) > -1)
	{
		CAT_draw_sprite(&icon_uv_sprite, 0, cursor_x, cursor_y);
		cursor_x += icon_uv_sprite.width + 4;
	}
	if(CAT_room_find(prop_purifier_item) > -1)
	{
		CAT_draw_sprite(&icon_pure_sprite, 0, cursor_x, cursor_y);
		cursor_x += icon_pure_sprite.width + 4;
	}
}