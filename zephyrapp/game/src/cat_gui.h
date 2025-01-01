#pragma once

#include "cat_math.h"
#include "cat_sprite.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_GLYPH_WIDTH 8
#define CAT_GLYPH_HEIGHT 12


//////////////////////////////////////////////////////////////////////////
// RENDERING

typedef enum CAT_gui_flag
{
	CAT_GUI_DEFAULT,
	CAT_GUI_NO_BORDER,
	CAT_GUI_WRAP_TEXT,
	CAT_GUI_TIGHT
} CAT_gui_flag;

typedef struct CAT_gui
{
	CAT_gui_flag flags;

	CAT_ivec2 start;
	CAT_ivec2 shape;
	CAT_ivec2 cursor;

	int margin;
	int pad;
	int channel_height;
} CAT_gui;
extern CAT_gui gui;

void CAT_gui_set_flag(CAT_gui_flag flag);
bool CAT_gui_consume_flag(CAT_gui_flag flag);

void CAT_gui_panel(CAT_ivec2 start, CAT_ivec2 shape);
void CAT_gui_line_break();

void CAT_gui_text(const char* text);
void CAT_gui_image(CAT_sprite* sprite, int frame_idx);
void CAT_gui_div(const char* text);
void CAT_gui_textf(const char* fmt, ...);
void CAT_gui_title(bool tabs, CAT_sprite* a_action, CAT_sprite* b_action, const char* fmt, ...);

extern bool keyboard_open;
bool CAT_gui_keyboard();
bool CAT_gui_harvest_keyboard(char* destination);
