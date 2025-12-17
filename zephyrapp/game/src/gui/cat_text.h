#pragma once

#include <stdint.h>

#define CAT_TEXT_RAW_BUFFER_CAPACITY 1024
#define CAT_TEXT_SKIP_BUFFER_CAPACITY 32
#define CAT_TEXT_BREAK_BUFFER_CAPACITY 64
#define CAT_TEXT_COLOUR_BUFFER_CAPACITY 16

typedef enum
{
	CAT_TEXT_ALIGNMENT_LEFT,
	CAT_TEXT_ALIGNMENT_CENTER,
	CAT_TEXT_ALIGNMENT_RIGHT
} CAT_text_alignment;


//////////////////////////////////////////////////////////////////////////
// DRAWING

void CAT_draw_text2(int x, int y, int scale, uint16_t colour, const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// TEXT BOX

void CAT_reset_text_box();
void CAT_set_text_box(int x0, int y0, int x1, int y1);
void CAT_set_text_box_alignment(int alignment);

int CAT_get_text_box_cursor_x();
int CAT_get_text_box_cursor_y();
void CAT_set_text_box_cursor_x(int x);
void CAT_set_text_box_cursor_y(int y);

void CAT_text_box_draw(int scale, uint16_t colour, const char* fmt, ...);

