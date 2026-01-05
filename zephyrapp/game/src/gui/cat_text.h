#pragma once

#include <stdint.h>
#include "cat_render.h"

#define CAT_TEXT_RAW_BUFFER_CAPACITY 1024
#define CAT_TEXT_SKIP_BUFFER_CAPACITY 32
#define CAT_TEXT_BREAK_BUFFER_CAPACITY 64
#define CAT_TEXT_COLOUR_BUFFER_CAPACITY 16

typedef enum
{
	CAT_TEXT_ALIGNMENT_LEFT,
	CAT_TEXT_ALIGNMENT_CENTER,
	CAT_TEXT_ALIGNMENT_RIGHT,
	CAT_TEXT_ALIGNMENT_VERTICAL
} CAT_text_alignment;


//////////////////////////////////////////////////////////////////////////
// DRAWING

void CAT_draw_text(int x, int y, int scale, uint16_t colour, const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// TEXT BOX

void CAT_reset_text_box();
void CAT_set_text_box(int x0, int y0, int x1, int y1);
void CAT_set_text_box_alignment(int alignment);

int CAT_get_text_box_cursor_x();
int CAT_get_text_box_cursor_y();
void CAT_set_text_box_cursor_x(int x);
void CAT_set_text_box_cursor_y(int y);
void CAT_text_box_shift_cursor(int dx, int dy);
void CAT_text_box_reset_x();
void CAT_text_box_reset_y();
void CAT_text_box_newline(int scale);

void CAT_text_box_draw(int scale, uint16_t colour, const char* fmt, ...);
void CAT_text_box_draw_sprite(const CAT_sprite* sprite, int frame_idx);

