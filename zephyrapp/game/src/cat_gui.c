#include "cat_gui.h"

#include "cat_sprite.h"

//////////////////////////////////////////////////////////////////////////
// RENDERING

CAT_gui gui;

void CAT_gui_init(int* tiles, int* glyphs)
{
	for(int i = 0; i < 9; i++)
	{
		gui.tiles[i] = tiles[i];
	}
	for(int i = 0; i < 91; i++)
	{
		gui.glyphs[i] = glyphs[i];
	}

	gui.start = (CAT_ivec2) {0, 0};
	gui.shape = (CAT_ivec2) {15, 20};

	gui.cursor = (CAT_ivec2) {0, 0};
	gui.channel_height = 0; 
}

void CAT_gui_row(int stage)
{
	CAT_draw_sprite(gui.cursor.x, gui.cursor.y, gui.tiles[stage*3+0]);
	gui.cursor.x += CAT_TILE_SIZE;
	for(int col = 1; col < gui.shape.x-1; col++)
	{
		CAT_draw_sprite(gui.cursor.x, gui.cursor.y, gui.tiles[stage*3+1]);
		gui.cursor.x += CAT_TILE_SIZE;
	}
	CAT_draw_sprite(gui.cursor.x, gui.cursor.y, gui.tiles[stage*3+2]);
	gui.cursor.x += CAT_TILE_SIZE;

	gui.cursor.y += CAT_TILE_SIZE;
	gui.cursor.x = gui.start.x;
}

void CAT_gui_panel(CAT_ivec2 start, CAT_ivec2 shape)
{
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
	CAT_gui_open_channel(CAT_GLYPH_HEIGHT);

	for(const char* c = text; *c != '\0'; c++)
	{
		if(*c == '\n')
		{
			CAT_gui_line_break();
			continue;
		}

		CAT_draw_sprite(gui.cursor.x, gui.cursor.y, gui.glyphs[(*c)-' ']);
		gui.cursor.x += CAT_GLYPH_WIDTH;
	}

	gui.cursor.x += CAT_GUI_PAD_X;
}

void CAT_gui_image(int sprite_id)
{
	CAT_sprite sprite = atlas.table[sprite_id];
	CAT_gui_open_channel(sprite.height);

	CAT_draw_sprite(gui.cursor.x, gui.cursor.y, sprite_id);
	gui.cursor.x += sprite.width;

	gui.cursor.x += CAT_GUI_PAD_X;
}