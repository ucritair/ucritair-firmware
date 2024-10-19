#ifndef CAT_GUI_H
#define CAT_GUI_H

#include "cat_texture.h"
#include "cat_math.h"

typedef struct CAT_gui
{
	int tiles[9];
	int glyphs[91];

	CAT_ivec2 start;
	CAT_ivec2 shape;
	int margin;

	CAT_ivec2 cursor;
	CAT_ivec2 cursor_last;
	int channel_width;
} CAT_gui;

void CAT_gui_row(CAT_gui* gui, CAT_atlas* atlas, CAT_texture* frame, int stage)
{
	CAT_draw_sprite(frame, gui->cursor.x, gui->cursor.y, atlas, gui->tiles[stage*3+0], CAT_DRAW_MODE_DEFAULT);
	gui->cursor.x += 16;
	for(int col = 1; col < gui->shape.x-1; col++)
	{
		CAT_draw_sprite(frame, gui->cursor.x, gui->cursor.y, atlas, gui->tiles[stage*3+1], CAT_DRAW_MODE_DEFAULT);
		gui->cursor.x += 16;
	}
	CAT_draw_sprite(frame, gui->cursor.x, gui->cursor.y, atlas, gui->tiles[stage*3+2], CAT_DRAW_MODE_DEFAULT);
	gui->cursor.x += 16;

	gui->cursor.y += 16;
	gui->cursor.x = gui->start.x;
}

void CAT_gui_panel(CAT_gui* gui, CAT_atlas* atlas, CAT_texture* frame, CAT_ivec2 start, CAT_ivec2 shape)
{
	gui->start = start;
	gui->shape = shape;
	gui->cursor = start;

	CAT_gui_row(gui, atlas, frame, 0);
	for(int row = 1; row < gui->shape.y-1; row++)
	{
		CAT_gui_row(gui, atlas, frame, 1);
	}
	CAT_gui_row(gui, atlas, frame, 2);

	gui->cursor = gui->start;
	gui->cursor.y += gui->margin + 6;
	gui->cursor.x += gui->margin;
}

void CAT_gui_line_break(CAT_gui* gui)
{
	gui->cursor_last = gui->cursor;
	gui->cursor.y += gui->channel_width/2 + 10;
	gui->cursor.x = gui->start.x + gui->margin;
	gui->channel_width = 16;
}

void CAT_gui_same_line(CAT_gui* gui)
{
	gui->cursor = gui->cursor_last;
	gui->cursor.x += gui->margin;
}

void CAT_gui_text(CAT_gui* gui, CAT_atlas* atlas, CAT_texture* frame, const char* text)
{
	for(const char* c = text; *c != '\0'; c++)
	{
		if(*c == '\n')
		{
			CAT_gui_line_break(gui);
			continue;
		}

		CAT_draw_sprite(frame, gui->cursor.x, gui->cursor.y, atlas, gui->glyphs[(*c)-' '], CAT_DRAW_MODE_CENTER_Y);
		gui->cursor.x += gui->margin;
	}
	
	CAT_gui_line_break(gui);
}

void CAT_gui_image(CAT_gui* gui, CAT_atlas* atlas, CAT_texture* frame, int key)
{
	CAT_sprite sprite = atlas->map[key];
	CAT_draw_sprite(frame, gui->cursor.x, gui->cursor.y, atlas, key, CAT_DRAW_MODE_CENTER_Y);
	gui->cursor.x += sprite.width / 2 + gui->margin;
	gui->channel_width = sprite.width;
	
	CAT_gui_line_break(gui);
}

typedef struct CAT_menu
{
	int length;
	int window;

	int idx;
	int base;
} CAT_gui_list;

void CAT_menu_init(CAT_menu* menu, int length, int window)
{
	menu->length = length;
	menu->window = window;
	menu->idx = 0;
	menu->base = 0;
}

void CAT_menu_shift(CAT_menu* menu, int shift)
{
	menu->idx += shift;	
	menu->idx = clamp(menu->idx, 0, menu->length-1);

	int overshoot = menu->idx - menu->base - menu->length + 1;
	if(overshoot > 0)
	{
		menu->base += overshoot;
	}
	else if(overshoot <= -menu->length)
	{
		menu->base -= overshoot;
	}
}

#endif
