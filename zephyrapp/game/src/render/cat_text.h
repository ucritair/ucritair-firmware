#pragma once

#include "cat_render.h"
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_TEXT_LINE_HEIGHT (CAT_GLYPH_HEIGHT + 2)
#define CAT_TEXT_MAX_LINES (CAT_LCD_SCREEN_H / CAT_TEXT_LINE_HEIGHT)

static const char* example_text =
"I must not fear."
"Fear is the mind-killer."
"Fear is the little-death that brings total obliteration."
"I will face my fear."
"I will permit it to pass over me and through me."
"And when it has gone past, I will turn the inner eye to see its path."
"Where the fear has gone there will be nothing."
"Only I will remain";


//////////////////////////////////////////////////////////////////////////
// WRAPPING

void break_list_init(const char* txt, int line_width, int scale);
bool break_list_lookup(int idx);


//////////////////////////////////////////////////////////////////////////
// DRAWING

typedef enum
{
	CAT_TEXT_FLAG_DEFAULT = 1,
	CAT_TEXT_FLAG_WRAP = 2
} CAT_text_flag;

void CAT_push_text_flags(int flags);
void CAT_push_text_line_width(int width);
void CAT_push_text_colour(uint16_t colour);
void CAT_push_text_scale(uint8_t scale);

int CAT_draw_text(int x, int y, const char* text);
int CAT_draw_textf(int x, int y, const char* fmt, ...);