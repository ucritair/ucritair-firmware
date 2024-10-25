#include "cat_gui.h"

#include "cat_sprite.h"
#include "cat_math.h"

void CAT_gui_init(CAT_gui* gui, int* tiles, int* glyphs)
{
	for(int i = 0; i < 9; i++)
	{
		gui->tiles[i] = tiles[i];
	}
	for(int i = 0; i < 91; i++)
	{
		gui->glyphs[i] = glyphs[i];
	}

	gui->start = (CAT_ivec2) {0, 0};
	gui->shape = (CAT_ivec2) {15, 20};
	gui->margin = 8;

	gui->cursor = (CAT_ivec2) {14, 8};
	gui->cursor_last = (CAT_ivec2) {14, 8};
	gui->channel_height = 0; 
}

void CAT_gui_row(CAT_gui* gui, int stage)
{
	spriter.mode = CAT_DRAW_MODE_DEFAULT;

	CAT_draw_sprite(gui->cursor.x, gui->cursor.y, gui->tiles[stage*3+0]);
	gui->cursor.x += 16;
	for(int col = 1; col < gui->shape.x-1; col++)
	{
		CAT_draw_sprite(gui->cursor.x, gui->cursor.y, gui->tiles[stage*3+1]);
		gui->cursor.x += 16;
	}
	CAT_draw_sprite(gui->cursor.x, gui->cursor.y, gui->tiles[stage*3+2]);
	gui->cursor.x += 16;

	gui->cursor.y += 16;
	gui->cursor.x = gui->start.x;
}

void CAT_gui_panel(CAT_gui* gui, CAT_ivec2 start, CAT_ivec2 shape)
{
	gui->start = start;
	gui->shape = shape;
	gui->cursor = start;

	CAT_gui_row(gui, 0);
	for(int row = 1; row < gui->shape.y-1; row++)
	{
		CAT_gui_row(gui, 1);
	}
	CAT_gui_row(gui, 2);

	gui->cursor = gui->start;
	gui->cursor.y += gui->margin + 6;
	gui->cursor.x += gui->margin;
}

void CAT_gui_line_break(CAT_gui* gui)
{
	gui->cursor_last = gui->cursor;
	gui->cursor.y += gui->channel_height/2 + 10;
	gui->cursor.x = gui->start.x + gui->margin;
	gui->channel_height = 16;
}

void CAT_gui_same_line(CAT_gui* gui)
{
	gui->cursor = gui->cursor_last;
	gui->cursor.x += gui->margin;
}

void CAT_gui_text(CAT_gui* gui, const char* text)
{
	spriter.mode = CAT_DRAW_MODE_CENTER_Y;

	for(const char* c = text; *c != '\0'; c++)
	{
		if(*c == '\n')
		{
			CAT_gui_line_break(gui);
			continue;
		}

		CAT_draw_sprite(gui->cursor.x, gui->cursor.y, gui->glyphs[(*c)-' ']);
		gui->cursor.x += gui->margin;
	}
	
	CAT_gui_line_break(gui);
}

void CAT_gui_image(CAT_gui* gui, int key)
{
	spriter.mode = CAT_DRAW_MODE_CENTER_Y;

	CAT_sprite sprite = atlas.table[key];
	if(sprite.width > 20)
	{
		gui->cursor.y += (sprite.width-20)/2;
	}
	CAT_draw_sprite(gui->cursor.x, gui->cursor.y, key);
	gui->cursor.x += sprite.width / 2 + gui->margin;
	gui->channel_height = sprite.height;
	
	CAT_gui_line_break(gui);
}

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

	int overshoot = menu->idx - menu->base - menu->window + 1;
	if(overshoot > 0)
	{
		menu->base++;
	}
	else if(overshoot <= -menu->window)
	{
		menu->base--;
	}
}
