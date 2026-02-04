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
// ANALYSIS

void CAT_TXTAN_measure
(
	int x0, int y0, int x1, int y1,
	int scale, const char* text,
	int* w_out, int* h_out
);


//////////////////////////////////////////////////////////////////////////
// DRAWING

void CAT_draw_text(int x, int y, int scale, uint16_t colour, const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// TEXT BOX

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


//////////////////////////////////////////////////////////////////////////
// LEGACY

typedef enum
{
	CAT_TEXT_FLAG_NONE = 0,
	CAT_TEXT_FLAG_WRAP = (1 << 0),
	CAT_TEXT_FLAG_CENTER = (1 << 1), 
	CAT_TEXT_FLAG_VERTICAL = (1 << 2)
} CAT_text_flag_depr;

void CAT_set_text_flags_depr(int flags);
void CAT_set_text_colour_depr(uint16_t colour);
void CAT_set_text_scale_depr(uint8_t scale);
void CAT_set_text_mask_depr(int x0, int y0, int x1, int y1);

int CAT_draw_text_depr(int x, int y, const char* text);
int CAT_draw_textf_depr(int x, int y, const char* fmt, ...);
size_t CAT_get_drawn_strlen_depr();

#define CAT_FLOAT_FMT "%d.%2.2u"
#define CAT_FMT_FLOAT(f) (int) (f), ((unsigned)(100 * ((f) - (int) (f))) % 100)
#define CAT_LINE_CAPACITY(l, r, w) ((CAT_LCD_SCREEN_W - (l + r))/w)