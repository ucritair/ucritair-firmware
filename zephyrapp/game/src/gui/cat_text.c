#include "cat_gui.h"

#include "sprite_assets.h"
#include <stdarg.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
// WRAPPING

int break_list[CAT_TEXT_MAX_LINES];
int break_count = 0;

int wrdlen(const char* txt, int idx)
{
	int length = 0;
	for(int i = idx; txt[i] != ' ' && txt[i] != '\n' && txt[i] != '\0'; i++)
	{
		if(txt[i] == '\t')
			length += 4;
		else
			length += 1;
	}
	return length;
}

void break_list_init(const char* txt, int line_width, int scale)
{
	break_count = 0;

	int L = strlen(txt);
	int idx = 0;
	int slack = line_width;

	while(txt[idx] != '\0' && idx < L)
	{
		int l = wrdlen(txt, idx);
		int word_width = l * CAT_GLYPH_WIDTH * scale;
		int space_width = CAT_GLYPH_WIDTH * scale;

		if ((word_width + space_width) > slack)
		{
			break_list[break_count] = idx;
			break_count += 1;

			slack = line_width - word_width;
		}
		else
		{
			slack -= (word_width + space_width);
		}

		idx += l+1;
	}
}

bool break_list_lookup(int idx)
{
	for(int i = 0; i < break_count; i++)
	{
		if(break_list[i] == idx)
			return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
// DRAWING

static int text_flags = CAT_TEXT_FLAG_DEFAULT;
static int text_line_width = CAT_LCD_SCREEN_W;
static uint16_t text_colour = CAT_BLACK;
static uint8_t text_scale = 1;

void CAT_push_text_flags(int flags)
{
	text_flags = flags;
}

int consume_text_flags()
{
	int value = text_flags;
	text_flags = CAT_TEXT_FLAG_DEFAULT;
	return value;
}

void CAT_push_text_line_width(int width)
{
	text_line_width = width;
}

int consume_text_line_width()
{
	int value = text_line_width;
	text_line_width = CAT_LCD_SCREEN_W;
	return value;
}

void CAT_push_text_colour(uint16_t colour)
{
	text_colour = colour;
}

uint16_t consume_text_colour()
{
	uint16_t value = text_colour;
	text_colour = CAT_BLACK;
	return value;
}

void CAT_push_text_scale(uint8_t scale)
{
	text_scale = scale;
}

uint8_t consume_text_scale()
{
	uint8_t value = text_scale;
	text_scale = 1;
	return value;
}

int CAT_draw_text(int x, int y, const char* text)
{
	int flags = consume_text_flags();
	int line_width = consume_text_line_width();
	uint16_t colour = consume_text_colour();
	int scale = consume_text_scale();

	bool wrap = (flags & CAT_TEXT_FLAG_WRAP) > 0;
	if(wrap)
		break_list_init(text, line_width, scale);

	const char* glyph_ptr = text; int glyph_idx = 0;
	int cursor_x = x;
	int cursor_y = y;

	while (*glyph_ptr != '\0')
	{
		if(*glyph_ptr == '\n')
		{
			cursor_x = x;
			cursor_y += (CAT_GLYPH_HEIGHT + 2) * scale;
			glyph_ptr++; glyph_idx++;
			continue;
		}
		else if(wrap)
		{
			if(break_list_lookup(glyph_idx))
			{
				cursor_x = x;
				cursor_y += (CAT_GLYPH_HEIGHT + 2) * scale;
			}
		}

		if(*glyph_ptr == '\t')
		{
			cursor_x += CAT_GLYPH_WIDTH * 4 * scale;
			glyph_ptr++; glyph_idx++;
			continue;
		}

		CAT_push_draw_colour(colour);
		CAT_push_draw_scale(scale);
		CAT_draw_sprite(&glyph_sprite, *glyph_ptr, cursor_x, cursor_y);
		cursor_x += CAT_GLYPH_WIDTH * scale;
		glyph_ptr++; glyph_idx++;
	}

	return cursor_y;
}

char textf_buffer[512];

int CAT_draw_textf(int x, int y, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(textf_buffer, 512, fmt, args);
	va_end(args);

	return CAT_draw_text(x, y, textf_buffer);
}