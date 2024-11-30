#include "cat_gui.h"

#include "cat_sprite.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "cat_machine.h"
#include <math.h>
#include <ctype.h>

//////////////////////////////////////////////////////////////////////////
// RENDERING

CAT_gui gui =
{
	.start = (CAT_ivec2) {0, 0},
	.shape = (CAT_ivec2) {0, 0},
	.margin = 8,
	.pad_x = 4,
	.pad_y = 4,

	.cursor = (CAT_ivec2) {0, 0},
	.channel_height = 0
};

void CAT_gui_panel(CAT_ivec2 start, CAT_ivec2 shape)
{
	start.x *= 16;
	start.y *= 16;
	gui.start = start;
	gui.shape = shape;
	gui.cursor = start;
	
	spriter.mode = CAT_DRAW_MODE_DEFAULT;

	CAT_fillberry(start.x, start.y, shape.x * 16, shape.y * 16, 0xFFFF);
	CAT_strokeberry(start.x, start.y, shape.x * 16, shape.y * 16, 0x0000);

	gui.cursor = gui.start;
	gui.cursor.y += gui.margin;
	gui.cursor.x += gui.margin;
	gui.channel_height = 0;

	spriter.mode = CAT_DRAW_MODE_CENTER_Y;
}

void CAT_gui_panel_tight(CAT_ivec2 start, CAT_ivec2 shape)
{
	CAT_gui_panel(start, shape);
	gui.cursor.y -= gui.margin / 2;
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
	gui.cursor.y +=
	(gui.channel_height > 0 ?
	gui.channel_height / 2 :
	CAT_GLYPH_HEIGHT) +
	gui.pad_y;
	gui.cursor.x = gui.start.x + gui.margin;
	gui.channel_height = 0;
}

void CAT_gui_text(const char* text)
{
	const char* c = text;
	while(*c != '\0')
	{
		if(*c == '\n')
		{
			CAT_gui_line_break();
			c++;
			continue;
		}
		if(*c == '\t')
		{
			gui.cursor.x += CAT_GLYPH_WIDTH * 4;
			c++;
			continue;
		}

		CAT_gui_open_channel(CAT_GLYPH_HEIGHT);
		CAT_draw_sprite(glyph_sprite, *c-' ', gui.cursor.x, gui.cursor.y);
		gui.cursor.x += CAT_GLYPH_WIDTH;
		c++;
	}
}

void CAT_gui_text_wrap(const char* text)
{
	int x_lim = gui.start.x + (gui.shape.x) * CAT_TILE_SIZE - CAT_GLYPH_WIDTH - gui.margin;
	const char* c = text;

	while(*c != '\0')
	{
		if(gui.cursor.x >= x_lim && !isspace(*(c+1)))
		{
			if(!isspace(*c) && !isspace(*(c-1)))
				CAT_draw_sprite(glyph_sprite, '-'-' ', gui.cursor.x, gui.cursor.y);
			CAT_gui_line_break();	
			if(isspace(*c))
				c++;
		}

		if(*c == '\n')
		{
			CAT_gui_line_break();
			c++;
			continue;
		}
		if(*c == '\t')
		{
			gui.cursor.x += CAT_GLYPH_WIDTH * 4;
			c++;
			continue;
		}

		CAT_gui_open_channel(CAT_GLYPH_HEIGHT);
		CAT_draw_sprite(glyph_sprite, *c-' ', gui.cursor.x, gui.cursor.y);
		gui.cursor.x += CAT_GLYPH_WIDTH;
		c++;
	}
}

void CAT_gui_image(int sprite_id, int frame_idx)
{
	CAT_sprite sprite = atlas.table[sprite_id];
	CAT_gui_open_channel(sprite.height);

	CAT_draw_sprite(sprite_id, frame_idx, gui.cursor.x, gui.cursor.y);
	gui.cursor.x += sprite.width;

	gui.cursor.x += gui.pad_x;
}

void CAT_gui_div(const char* text)
{
	CAT_gui_line_break();
	CAT_gui_open_channel(CAT_TILE_SIZE);
	if(strlen(text) == 0)
	{
		CAT_lineberry(0, gui.cursor.y, 240, gui.cursor.y, 0x0000);
	}
	else
	{
		CAT_gui_text(text);
		CAT_lineberry(gui.cursor.x + gui.pad_x, gui.cursor.y, 240-gui.margin, gui.cursor.y, 0x0000);
	}
	CAT_gui_line_break();
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

void CAT_gui_popup(const char* text)
{
	int lines = 0;
	int max_length = 0;
	int length = 0;
	for(const char* c = text; *c != '\0'; c++)
	{
		max_length = max(max_length, ++length);
		if(*c == '\n')
		{
			lines += 1;
			length = 0;
		}
	}
	
	int dialog_width = max_length * CAT_GLYPH_WIDTH + (CAT_TILE_SIZE + gui.margin * 2);
	dialog_width = round((float) dialog_width / (float) CAT_TILE_SIZE);
	int dialog_height = lines * (CAT_GLYPH_HEIGHT + gui.pad_y) + (CAT_TILE_SIZE + gui.margin * 2);
	dialog_height = round((float) dialog_height / (float) CAT_TILE_SIZE);

	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {dialog_width, dialog_height});
	CAT_gui_text(text);
}
