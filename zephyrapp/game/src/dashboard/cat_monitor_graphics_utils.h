#pragma once

#include <stdint.h>
#include "cat_render.h"
#include "cat_curves.h"
#include <stdarg.h>
#include <stdio.h>
#include "cat_gui.h"
#include "sprite_assets.h"
#include "cat_air.h"
#include "cat_monitors.h"
#include "cat_crisis.h"
#include "cat_gizmos.h"
#include "cat_graph.h"

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

static int vert_text(int x, int y, uint16_t c, const char* text)
{
	const char* g = text;
	x -= CAT_GLYPH_WIDTH/2;
	
	while(*g != '\0')
	{
		CAT_set_sprite_colour(CAT_WHITE);
		CAT_draw_sprite(&glyph_sprite, *g, x, y);
		g++;
		y += CAT_GLYPH_HEIGHT + 1;
	}

	return y + CAT_TEXT_LINE_HEIGHT;
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