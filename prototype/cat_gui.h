#ifndef CAT_GUI_H
#define CAT_GUI_H

#include "cat_math.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_GLYPH_WIDTH 8
#define CAT_GLYPH_HEIGHT 12
#define CAT_GUI_MARGIN 8
#define CAT_GUI_PAD_X 4
#define CAT_GUI_PAD_Y 4


//////////////////////////////////////////////////////////////////////////
// RENDERING

typedef struct CAT_gui
{
	int tiles[9];
	int glyphs[91];

	CAT_ivec2 start;
	CAT_ivec2 shape;
	int margin;

	CAT_ivec2 cursor;
	CAT_ivec2 cursor_last;
	int channel_height;
} CAT_gui;

void CAT_gui_init(CAT_gui* gui, int* tiles, int* glyphs);
void CAT_gui_row(CAT_gui* gui, int stage);
void CAT_gui_panel(CAT_gui* gui, CAT_ivec2 start, CAT_ivec2 shape);
void CAT_gui_line_break(CAT_gui* gui);
void CAT_gui_same_line(CAT_gui* gui);
void CAT_gui_text(CAT_gui* gui, const char* text);
void CAT_gui_image(CAT_gui* gui, int sprite_id);

#endif
