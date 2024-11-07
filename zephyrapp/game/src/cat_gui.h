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

typedef enum CAT_text_mode
{
	CAT_TEXT_MODE_NORMAL,
	CAT_TEXT_MODE_STRIKETHROUGH
} CAT_text_mode;

typedef struct CAT_gui
{
	CAT_ivec2 start;
	CAT_ivec2 shape;
	int margin;

	CAT_ivec2 cursor;
	int channel_height;

	CAT_text_mode text_mode;
} CAT_gui;
extern CAT_gui gui;

void CAT_gui_init();
void CAT_gui_row(int stage);
void CAT_gui_panel(CAT_ivec2 start, CAT_ivec2 shape);
void CAT_gui_panel_tight(CAT_ivec2 start, CAT_ivec2 shape);
void CAT_gui_open_channel(int height);
void CAT_gui_line_break();
void CAT_gui_same_line();
void CAT_gui_text(const char* text);
void CAT_gui_image(int sprite_id, int frame_idx);
void CAT_gui_div(const char* text);

#endif
