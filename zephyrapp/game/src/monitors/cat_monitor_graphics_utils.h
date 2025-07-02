#pragma once

#include <stdint.h>
#include "cat_render.h"
#include "cat_curves.h"
#include <stdarg.h>
#include <stdio.h>
#include "cat_gui.h"
#include "sprite_assets.h"
#include "cat_aqi.h"
#include "cat_monitors.h"
#include "cat_crisis.h"

static void draw_diamond(int x, int y, int r)
{
	int x0 = x-r-1; int y0 = y-1;
	int x1 = x-1; int y1 = y-r-1;
	int x2 = x+r-1; int y2 = y-1;
	int x3 = x-1; int y3 = y+r-1;
	CAT_lineberry(x0+1, y0, x1, y1+1, CAT_RED);
	CAT_lineberry(x1+1, y1+1, x2, y2, CAT_RED);
	CAT_lineberry(x2, y2+1, x3+1, y3, CAT_RED);
	CAT_lineberry(x3, y3, x0+1, y0+1, CAT_RED);
}

static void draw_page_markers(int y, int pages, int page)
{
	int start_x = 120 - ((16 + 2) * pages) / 2;
	for(int i = 0; i < pages; i++)
	{
		int x = start_x + i * (16 + 2);
		if(i == CAT_MONITOR_PAGE_GAMEPLAY && CAT_AQ_is_crisis_ongoing())
		{				
			if(page == CAT_MONITOR_PAGE_GAMEPLAY)		
				CAT_set_sprite_colour(CAT_RED);
			CAT_draw_sprite(&ui_radio_button_diamond_sprite, page == i, x, y);
			if(CAT_pulse(0.25f))
				draw_diamond(x+8, y+8, 16);
		}
		else
		{
			CAT_set_sprite_colour(CAT_monitor_fg_colour());
			CAT_draw_sprite(&ui_radio_button_diamond_sprite, page == i, x, y);
		}
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
		CAT_set_sprite_colour(CAT_WHITE);
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

static void score_bar(int x, int y, int aqm)
{
	float subscore = 1-CAT_AQ_get_normalized_score(aqm);
	int total_width = 16*4;
	int filled_width = total_width * subscore;
	uint16_t colour = colour_score(1-subscore);

	CAT_discberry(x, y, 4, colour);
	CAT_lineberry(x+4, y, x+4+filled_width, y, colour);
	CAT_lineberry(x+4+filled_width, y, x+4+total_width, y, CAT_WHITE);
	CAT_discberry(x+4+total_width+4, y, 4, colour);
}

static int labeled_scoref(int x, int y, uint16_t c, int aqm, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(textf_buf, 32, fmt, args);
	va_end(args);

	CAT_set_text_colour(c);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT, CAT_AQM_titles[aqm]);
	x += strlen(CAT_AQM_titles[aqm]) * CAT_GLYPH_WIDTH + 8;

	CAT_set_text_scale(2);
	CAT_set_text_colour(c);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT*2, textf_buf);
	x += strlen(textf_buf) * CAT_GLYPH_WIDTH*2 + 8;

	const char* unit = CAT_get_AQM_unit_string(aqm);
	CAT_set_text_colour(c);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT, unit);
	x += strlen(unit) == 0 ? 4 : strlen(unit) * CAT_GLYPH_WIDTH + 12;

	int y_off = strlen(unit) == 0 ? -CAT_GLYPH_HEIGHT : -CAT_GLYPH_HEIGHT/2;
	score_bar(x, y+y_off, aqm);

	return y + CAT_GLYPH_HEIGHT*2 + 6;
}