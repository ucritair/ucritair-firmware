#include "cat_text.h"

#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "cat_render.h"
#include "cat_colours.h"
#include "sprite_assets.h"

//////////////////////////////////////////////////////////////////////////
// RAW BUFFER

static char raw_buffer[CAT_TEXT_RAW_BUFFER_CAPACITY];
static int raw_buffer_length = 0;

static void measure_raw_buffer()
{
	raw_buffer_length = strnlen(raw_buffer, sizeof(raw_buffer));
	raw_buffer[raw_buffer_length] = '\0';
}


//////////////////////////////////////////////////////////////////////////
// SKIP BUFFER

static struct skip_info
{
	int start;
	int end;
} skip_buffer[CAT_TEXT_SKIP_BUFFER_CAPACITY];
static int skip_buffer_length = 0;

static void clear_skip_bufffer()
{
	skip_buffer_length = 0;
}

static void buffer_skip(int start, int end)
{
	if(skip_buffer_length >= CAT_TEXT_SKIP_BUFFER_CAPACITY)
		return;
	skip_buffer[skip_buffer_length] = (struct skip_info)
	{
		.start = start,
		.end = end
	};
	skip_buffer_length += 1;
}

static struct skip_info* get_skip_info(int idx)
{
	for(int i = 0; i < skip_buffer_length; i++)
	{
		struct skip_info skip = skip_buffer[i];
		if(idx >= skip.start && idx < skip.end)
			return &skip_buffer[i];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// BREAK BUFFER

static int break_buffer[CAT_TEXT_BREAK_BUFFER_CAPACITY];
static int break_buffer_length = 0;

static void clear_break_bufffer()
{
	break_buffer_length = 0;
}

static void buffer_break(int idx)
{
	if(break_buffer_length >= CAT_TEXT_BREAK_BUFFER_CAPACITY)
		return;
	break_buffer[break_buffer_length] = idx;
	break_buffer_length += 1;
}

static bool get_break(int idx)
{
	for(int i = 0; i < break_buffer_length; i++)
	{
		if(break_buffer[i] == idx)
			return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
// COLOUR BUFFER

static struct colour_info
{
	int start;
	int end;
	uint16_t colour;
} colour_buffer[CAT_TEXT_COLOUR_BUFFER_CAPACITY];
static int colour_buffer_length = 0;

static void clear_colour_bufffer()
{
	colour_buffer_length = 0;
}

static void buffer_colour(int start, int end, uint16_t colour)
{
	if(colour_buffer_length >= CAT_TEXT_COLOUR_BUFFER_CAPACITY)
		return;
	colour_buffer[colour_buffer_length] = (struct colour_info)
	{
		.start = start,
		.end = end,
		.colour = colour
	};
	colour_buffer_length += 1;
}

static struct colour_info* get_colour_info(int idx)
{
	for(int i = 0; i < colour_buffer_length; i++)
	{
		struct colour_info colour = colour_buffer[i];
		if(idx >= colour.start && idx < colour.end)
			return &colour_buffer[i];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// PARSING

static int parse_idx = 0;

static bool consume_token(const char* token, int* start, int* end)
{
	int n = strlen(token);
	if(strncmp(&raw_buffer[parse_idx], token, n))
		return false;
	*start = parse_idx;
	parse_idx += n;
	*end = parse_idx;
	return true;
}

static bool consume_colour(int* start, int* end, uint16_t* colour)
{
	uint16_t c = atoi(&raw_buffer[parse_idx+1]);
	int n = strchr(&raw_buffer[parse_idx], '>') - &raw_buffer[parse_idx] + 1;
	*start = parse_idx;
	parse_idx += n;
	*end = parse_idx;
	*colour = c;
	return true;
}

static void buffer_colours()
{
	int start, end;
	struct colour_info colour_info;
	parse_idx = 0;

	while(parse_idx < raw_buffer_length)
	{
		if(consume_token("<c>", &start, &end))
		{
			buffer_skip(start, end);

			consume_colour(&start, &end, &colour_info.colour);
			colour_info.start = end;
			buffer_skip(start, end);
		}
		if(consume_token("</c>", &start, &end))
		{
			buffer_skip(start, end);

			colour_info.end = start;
			buffer_colour(colour_info.start, colour_info.end, colour_info.colour);
		}
		else
		{
			parse_idx += 1;
		}
	}
}

static bool is_whitespace()
{
	return
	raw_buffer[parse_idx] == ' ' ||
	raw_buffer[parse_idx] == '\t' ||
	raw_buffer[parse_idx] == '\n';
}

static void buffer_breaks(int glyph_width, int line_width)
{
	int slack = line_width;
	int last_pause = 0;
	parse_idx = 0;

	while(raw_buffer[parse_idx] != '\0')
	{
		if(is_whitespace())
			last_pause = parse_idx;
		if(!get_skip_info(parse_idx))
			slack -= glyph_width;
		if(slack <= 0 || raw_buffer[parse_idx] == '\n')
		{
			buffer_break(last_pause);
			slack = line_width - (parse_idx-last_pause) * glyph_width;
		}
		parse_idx++;
	}
}


//////////////////////////////////////////////////////////////////////////
// DRAWING

static int get_max_line_width(int scale)
{
	int idx = 0;
	int length = 0;
	int max_length = 0;
	while(raw_buffer[idx] != '\0')
	{
		if(!get_skip_info(idx))
			length++;
		if(raw_buffer[idx] == '\n' || get_break(idx))
		{
			max_length = CAT_max(max_length, length);
			length = 0;
		}
		idx++;
	}
	return CAT_max(max_length, length) * CAT_GLYPH_WIDTH * scale;
}

static int get_line_width_at(int idx, int scale)
{
	int length = 0;
	while
	(
		raw_buffer[idx] != '\0' &&
		raw_buffer[idx] != '\n' &&
		!get_break(idx)
	)
	{
		if(!get_skip_info(idx))
			length++;
		idx++;
	}
	return length * CAT_GLYPH_WIDTH * scale;
}

static int get_line_start(int idx, int scale, int x0, int x1, int alignment)
{
	int width = get_line_width_at(idx, scale);
	switch (alignment)
	{
		case CAT_TEXT_ALIGNMENT_LEFT: return x0;
		case CAT_TEXT_ALIGNMENT_CENTER: return (x0+x1) / 2 - width / 2;
		case CAT_TEXT_ALIGNMENT_RIGHT: return x1 - width;
		default: return x0;
	}
}

static void draw_text
(
	int x, int y,
	int x0, int y0,
	int x1, int y1,
	int alignment,
	int scale, uint16_t colour,
	int *x_out, int* y_out
)
{
	measure_raw_buffer();
	clear_skip_bufffer();
	clear_break_bufffer();
	clear_colour_bufffer();

	buffer_colours();

	if(x0 == x1)
	{
		x0 = x;
		x1 = x0 + get_max_line_width(scale);
	}
	if(y0 == y1)
	{
		y0 = y;
		y1 = CAT_INT_MAX;
	}
	buffer_breaks(CAT_GLYPH_WIDTH * scale, x1-x0);
	
	int idx = 0;
	int cursor_x = get_line_start(idx, scale, x0, x1, alignment);
	int cursor_y = y;

	while (raw_buffer[idx] != '\0')
	{
		if(get_skip_info(idx))
		{
			idx++;
			continue;
		}
		if(raw_buffer[idx] == '\n' || get_break(idx))
		{
			cursor_y += (CAT_GLYPH_HEIGHT + CAT_LEADING) * scale;
			idx++;
			cursor_x = get_line_start(idx, scale, x0, x1, alignment);
			continue;
		}

		struct colour_info* colour_info = get_colour_info(idx);
		if(colour_info)
			CAT_set_sprite_colour(colour_info->colour);
		else
			CAT_set_sprite_colour(colour);
		
		CAT_set_sprite_scale(scale);
		CAT_draw_sprite(&glyph_sprite, raw_buffer[idx], cursor_x, cursor_y);
		cursor_x += CAT_GLYPH_WIDTH * scale;
		idx++;
	}

	if(x_out != NULL)
		*x_out = cursor_x;
	if(y_out != NULL)
		*y_out = cursor_y;
}

void CAT_draw_text2(int x, int y, int scale, uint16_t colour, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(raw_buffer, sizeof(raw_buffer), fmt, args);
	va_end(args);

	draw_text
	(
		x, y,
		-1, -1, -1, -1,
		CAT_TEXT_ALIGNMENT_LEFT,
		scale, colour,
		NULL, NULL
	);
}


//////////////////////////////////////////////////////////////////////////
// TEXT BOX

static int text_box_x0 = 0;
static int text_box_y0 = 0;
static int text_box_x1 = CAT_LCD_SCREEN_W;
static int text_box_y1 = CAT_LCD_SCREEN_H;
static int text_box_alignment = CAT_TEXT_ALIGNMENT_LEFT;

static int text_box_x = 0;
static int text_box_y = 0;

void CAT_reset_text_box()
{
	text_box_x0 = 0;
	text_box_y0 = 0;
	text_box_x1 = CAT_LCD_SCREEN_W;
	text_box_y1 = CAT_LCD_SCREEN_H;
	text_box_alignment = CAT_TEXT_ALIGNMENT_LEFT;

	text_box_x = 0;
	text_box_y = 0;
}

void CAT_set_text_box(int x0, int y0, int x1, int y1)
{
	text_box_x0 = x0;
	text_box_y0 = y0;
	text_box_x1 = x1 >= x0 ? x1 : x0;
	text_box_y1 = y1 >= y0 ? y1 : y0;

	text_box_x = x0;
	text_box_y = y0;
}

void CAT_set_text_box_alignment(int alignment)
{
	text_box_alignment = alignment;
}

int CAT_get_text_box_cursor_x()
{
	return text_box_x;
}

int CAT_get_text_box_cursor_y()
{
	return text_box_y;
}

void CAT_set_text_box_cursor_x(int x)
{
	text_box_x = x;
}

void CAT_set_text_box_cursor_y(int y)
{
	text_box_y = y;
}

void CAT_text_box_draw(int scale, uint16_t colour, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(raw_buffer, sizeof(raw_buffer), fmt, args);
	va_end(args);

	draw_text
	(
		text_box_x, text_box_y,
		text_box_x0, text_box_y0,
		text_box_x1, text_box_y1,
		text_box_alignment,
		scale, colour,
		&text_box_x, &text_box_y
	);
}