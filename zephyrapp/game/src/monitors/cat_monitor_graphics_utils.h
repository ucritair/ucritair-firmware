#pragma once

#include <stdint.h>
#include "cat_render.h"
#include "cat_curves.h"
#include <stdarg.h>
#include <stdio.h>
#include "cat_gui.h"
#include "sprite_assets.h"

static void draw_page_markers(int y, int pages, int page)
{
	int start_x = 120 - ((16 + 2) * pages) / 2;
	for(int i = 0; i < pages; i++)
	{
		int x = start_x + i * (16 + 2);
		CAT_draw_sprite(&ui_radio_button_diamond_sprite, page == i, x, y);
	}
}

static void draw_subpage_markers(int y, int pages, int page)
{
	int r = 2;
	int pad = 8;
	int start_x = 120 - (((2*r) * pages) + (pad * (pages-1))) / 2;
	int x = start_x;

	for(int i = 0; i < pages; i++)
	{
		if(i == page)
			CAT_discberry(x, y, r, CAT_WHITE);
		else
			CAT_circberry(x, y, r, CAT_WHITE);
		x += 2 * r + pad;
	}
}

static uint16_t score_colours[3] =
{
	0xb985, // BAD
	0xf5aa, // MID
	0xd742, // GOOD
};

static uint16_t colour_score(float t)
{
	t = CAT_ease_inout_quad(t);

	float x = t * 2;
	int idx = (int) x;
	float frac = x - idx;
	uint16_t colour = CAT_RGB24216
	(
		CAT_RGB24_lerp
		(
			CAT_RGB16224(score_colours[idx]),
			CAT_RGB16224(score_colours[idx+1]),
			frac
		)
	);
	return colour;
}

static char textf_buf[32];

static int center_textf(int x, int y, int scale, uint16_t c, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(textf_buf, 32, fmt, args);
	va_end(args);

	int text_width = strlen(textf_buf) * CAT_GLYPH_WIDTH * scale;
	int left_x = x - text_width / 2;
	
	int text_height = CAT_GLYPH_HEIGHT * scale;
	int center_y = y - text_height / 2;

	CAT_set_text_scale(scale);
	CAT_set_text_colour(c);
	CAT_draw_text(left_x, center_y, textf_buf);

	return y + text_height;
}

static void vert_text(int x, int y, uint16_t c, const char* text)
{
	const char* g = text;
	x -= CAT_GLYPH_WIDTH/2;
	
	while(*g != '\0')
	{
		CAT_set_draw_colour(CAT_WHITE);
		CAT_draw_sprite(&glyph_sprite, *g, x, y);
		g++;
		y += CAT_GLYPH_HEIGHT + 1;
	}
}

static int underline(int x, int y, int scale, uint16_t c, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(textf_buf, 32, fmt, args);
	va_end(args);

	int text_width = strlen(textf_buf) * CAT_GLYPH_WIDTH * scale;
	int left_x = x - text_width / 2 - 8;
	int right_x = x + text_width / 2 + 8;

	CAT_discberry(left_x, y, 2, c);
	CAT_lineberry(left_x, y, right_x, y, c);
	CAT_discberry(right_x, y, 2, c);

	return y + 4;
}