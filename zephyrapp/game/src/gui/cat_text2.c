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
		if(slack <= 0)
		{
			buffer_break(last_pause);
			slack = line_width - (parse_idx-last_pause);
		}
		parse_idx++;
	}
}


//////////////////////////////////////////////////////////////////////////
// DRAWING

static void draw_text(int x, int y)
{
	measure_raw_buffer();
	clear_skip_bufffer();
	clear_break_bufffer();
	clear_colour_bufffer();

	buffer_colours();
	buffer_breaks(CAT_GLYPH_WIDTH, CAT_LCD_FRAMEBUFFER_W-x);

	const char* glyph_ptr = raw_buffer; int glyph_idx = 0;
	int cursor_x = x;
	int cursor_y = y;

	while (*glyph_ptr != '\0')
	{
		if(get_skip_info(glyph_idx))
		{
			glyph_ptr++; glyph_idx++;
			continue;
		}
		if(*glyph_ptr == '\n' || get_break(glyph_idx))
		{
			cursor_x = x;
			cursor_y += 14;
			glyph_ptr++; glyph_idx++;
			continue;
		}

		struct colour_info* colour_info = get_colour_info(glyph_idx);
		if(colour_info)
			CAT_set_sprite_colour(colour_info->colour);
		else
			CAT_set_sprite_colour(CAT_BLACK);
		
		CAT_draw_sprite(&glyph_sprite, *glyph_ptr, cursor_x, cursor_y);
		cursor_x += 8;
		glyph_ptr++; glyph_idx++;
	}
}

void CAT_draw_text2(int x, int y, const char* text)
{
	strncpy(raw_buffer, text, sizeof(raw_buffer));
	draw_text(x, y);
}

void CAT_draw_textf2(int x, int y, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(raw_buffer, sizeof(raw_buffer), fmt, args);
	va_end(args);
	draw_text(x, y);
}