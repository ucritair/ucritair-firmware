#include "cat_gui.h"

#include "cat_sprite.h"

//////////////////////////////////////////////////////////////////////////
// RENDERING

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

	gui->cursor = (CAT_ivec2) {0, 0};
	gui->cursor_last = (CAT_ivec2) {0, 0};
	gui->channel_height = CAT_GLYPH_HEIGHT; 
}

void CAT_gui_row(CAT_gui* gui, int stage)
{
	CAT_draw_sprite(gui->cursor.x, gui->cursor.y, gui->tiles[stage*3+0]);
	gui->cursor.x += CAT_TILE_SIZE;
	for(int col = 1; col < gui->shape.x-1; col++)
	{
		CAT_draw_sprite(gui->cursor.x, gui->cursor.y, gui->tiles[stage*3+1]);
		gui->cursor.x += CAT_TILE_SIZE;
	}
	CAT_draw_sprite(gui->cursor.x, gui->cursor.y, gui->tiles[stage*3+2]);
	gui->cursor.x += CAT_TILE_SIZE;

	gui->cursor.y += CAT_TILE_SIZE;
	gui->cursor.x = gui->start.x;
}

void CAT_gui_panel(CAT_gui* gui, CAT_ivec2 start, CAT_ivec2 shape)
{
	gui->start = start;
	gui->shape = shape;
	gui->cursor = start;
	
	spriter.mode = CAT_DRAW_MODE_DEFAULT;

	CAT_gui_row(gui, 0);
	for(int row = 1; row < gui->shape.y-1; row++)
	{
		CAT_gui_row(gui, 1);
	}
	CAT_gui_row(gui, 2);

	gui->cursor = gui->start;
	gui->cursor.y += CAT_GUI_MARGIN + CAT_GLYPH_HEIGHT / 2;
	gui->cursor.x += CAT_GUI_MARGIN;
}

void CAT_gui_line_break(CAT_gui* gui)
{
	gui->cursor_last = gui->cursor;
	gui->cursor.y += gui->channel_height / 2 + CAT_GLYPH_HEIGHT / 2 + CAT_GUI_PAD_Y;
	gui->cursor.x = gui->start.x + CAT_GUI_MARGIN;
	gui->channel_height = CAT_GLYPH_HEIGHT + CAT_GUI_PAD_Y;
}

void CAT_gui_same_line(CAT_gui* gui)
{
	gui->cursor = gui->cursor_last;
	gui->cursor.x += CAT_GUI_PAD_X;
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
		gui->cursor.x += CAT_GLYPH_WIDTH;
	}
	
	CAT_gui_line_break(gui);
}

void CAT_gui_image(CAT_gui* gui, int sprite_id)
{
	spriter.mode = CAT_DRAW_MODE_CENTER_Y;

	CAT_sprite sprite = atlas.table[sprite_id];
	if(sprite.height > gui->channel_height)
	{
		gui->cursor.y += (sprite.height-gui->channel_height) / 2;
	}
	CAT_draw_sprite(gui->cursor.x, gui->cursor.y, sprite_id);
	gui->cursor.x += sprite.width;
	gui->channel_height = sprite.height;
	
	CAT_gui_line_break(gui);
}
