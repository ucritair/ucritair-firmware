#include "cat_gui.h"
#include "cat_sprite.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
// RENDERING

CAT_gui gui;

void CAT_gui_init()
{
	gui.start = (CAT_ivec2) {0, 0};
	gui.shape = (CAT_ivec2) {15, 20};

	gui.cursor = (CAT_ivec2) {0, 0};
	gui.channel_height = 0;

	gui.text_mode = CAT_TEXT_MODE_NORMAL;
}

void CAT_gui_row(int stage)
{
	CAT_draw_sprite(panel_sprite, stage*3+0, gui.cursor.x, gui.cursor.y);
	gui.cursor.x += CAT_TILE_SIZE;
	for(int col = 1; col < gui.shape.x-1; col++)
	{
		CAT_draw_sprite(panel_sprite, stage*3+1, gui.cursor.x, gui.cursor.y);
		gui.cursor.x += CAT_TILE_SIZE;
	}
	CAT_draw_sprite(panel_sprite, stage*3+2, gui.cursor.x, gui.cursor.y);
	gui.cursor.x += CAT_TILE_SIZE;

	gui.cursor.y += CAT_TILE_SIZE;
	gui.cursor.x = gui.start.x;
}

void CAT_gui_panel(CAT_ivec2 start, CAT_ivec2 shape)
{
	start.x *= 16;
	start.y *= 16;
	gui.start = start;
	gui.shape = shape;
	gui.cursor = start;
	
	spriter.mode = CAT_DRAW_MODE_DEFAULT;

	CAT_gui_row(0);
	for(int row = 1; row < gui.shape.y-1; row++)
	{
		CAT_gui_row(1);
	}
	CAT_gui_row(2);

	gui.cursor = gui.start;
	gui.cursor.y += CAT_GUI_MARGIN;
	gui.cursor.x += CAT_GUI_MARGIN;
	gui.channel_height = 0;

	spriter.mode = CAT_DRAW_MODE_CENTER_Y;
}

void CAT_gui_panel_tight(CAT_ivec2 start, CAT_ivec2 shape)
{
	CAT_gui_panel(start, shape);
	gui.cursor.y -= CAT_GUI_MARGIN / 2;
}

void CAT_gui_open_channel(int height)
{
	if(gui.channel_height == 0)
		gui.cursor.y += height / 2;
	if(height > gui.channel_height)
		gui.channel_height = height;
}

void CAT_gui_line_break()
{
	gui.cursor.y += gui.channel_height / 2 + CAT_GUI_PAD_Y;
	gui.cursor.x = gui.start.x + CAT_GUI_MARGIN;
	gui.channel_height = 0;
}

void CAT_gui_text(const char* text)
{
	if(strlen(text) <= 0)
		return;

	CAT_gui_open_channel(CAT_GLYPH_HEIGHT);

	for(const char* c = text; *c != '\0'; c++)
	{
		if(*c == '\n')
		{
			CAT_gui_line_break();
			CAT_gui_open_channel(CAT_GLYPH_HEIGHT);
			continue;
		}

		CAT_draw_sprite(glyph_sprite, *c-' ', gui.cursor.x, gui.cursor.y);
		if(gui.text_mode == CAT_TEXT_MODE_STRIKETHROUGH)
			CAT_draw_sprite(strike_sprite, 0, gui.cursor.x, gui.cursor.y);
		gui.cursor.x += CAT_GLYPH_WIDTH;
	}

	gui.cursor.x += CAT_GUI_PAD_X;
}

void CAT_gui_image(int sprite_id, int frame_idx)
{
	CAT_sprite sprite = atlas.table[sprite_id];
	CAT_gui_open_channel(sprite.height);

	CAT_draw_sprite(sprite_id, frame_idx, gui.cursor.x, gui.cursor.y);
	gui.cursor.x += sprite.width;

	gui.cursor.x += CAT_GUI_PAD_X;
}

void CAT_gui_div(const char* text)
{
	int height = strlen(text) > 0 ? CAT_TILE_SIZE : 4;
	int ypad = strlen(text) > 0 ? CAT_GUI_PAD_Y : 0;

	CAT_gui_line_break();
	gui.cursor.y += ypad;

	CAT_gui_text(text);
	CAT_gui_open_channel(height);
	int n = (14 * CAT_TILE_SIZE - strlen(text) * CAT_GLYPH_WIDTH) / CAT_TILE_SIZE;
	for(int i = 0; i < n; i++)
	{
		CAT_draw_sprite(panel_sprite, 9, gui.cursor.x, gui.cursor.y);
		gui.cursor.x += CAT_TILE_SIZE;
	}

	CAT_gui_line_break();
	gui.cursor.y += ypad;
}

void CAT_gui_textf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char text[256];
	vsprintf(text, fmt, args);
	va_end(args);
	CAT_gui_text(text);
}
