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

void CAT_break_list_init(const char* txt, int line_width, int scale)
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

bool CAT_break_list_lookup(int idx)
{
	for(int i = 0; i < break_count; i++)
	{
		if(break_list[i] == idx)
			return true;
	}
	return false;
}

int CAT_break_list_count()
{
	return break_count;
}

int CAT_break_list_get(int idx)
{
	if(idx > 0 && idx < break_count)
		return break_list[idx];
	return -1;
}


//////////////////////////////////////////////////////////////////////////
// DRAWING

static int text_flags = CAT_TEXT_FLAG_NONE;
static uint16_t text_colour = CAT_BLACK;
static uint8_t text_scale = 1;
static CAT_rect text_mask = {{-1, -1}, {-1, -1}};

void CAT_set_text_flags(int flags)
{
	text_flags = flags;
}

int consume_text_flags()
{
	int value = text_flags;
	text_flags = CAT_TEXT_FLAG_NONE;
	return value;
}

void CAT_set_text_colour(uint16_t colour)
{
	text_colour = colour;
}

uint16_t consume_text_colour()
{
	uint16_t value = text_colour;
	text_colour = CAT_BLACK;
	return value;
}

void CAT_set_text_scale(uint8_t scale)
{
	text_scale = scale;
}

uint8_t consume_text_scale()
{
	uint8_t value = text_scale;
	text_scale = 1;
	return value;
}

void CAT_set_text_mask(int x0, int y0, int x1, int y1)
{
	text_mask = (CAT_rect){{x0, y0}, {x1, y1}};
}

CAT_rect consume_text_mask()
{
	CAT_rect value = text_mask;
	text_mask = (CAT_rect){{-1, -1}, {-1, -1}};
	return value;
}

int CAT_draw_text(int x, int y, const char* text)
{
	int flags = consume_text_flags();
	uint16_t colour = consume_text_colour();
	int scale = consume_text_scale();
	CAT_rect mask = consume_text_mask();

	int mask_x0 = mask.min.x == -1 ? x : mask.min.x;
	int mask_y0 = mask.min.y == -1 ? y : mask.min.y;
	int mask_x1 = mask.max.x == -1 ? CAT_LCD_SCREEN_W : mask.max.x;
	int mask_y1 = mask.max.y == -1 ? CAT_LCD_SCREEN_H : mask.max.y;
	mask_x0 = max(mask_x0, 0);
	mask_y0 = max(mask_y0, 0);
	mask_x1 = min(mask_x1, CAT_LCD_SCREEN_W);
	mask_y1 = min(mask_y1, CAT_LCD_SCREEN_H);

	bool wrap = (flags & CAT_TEXT_FLAG_WRAP) > 0;
	if(wrap)
		CAT_break_list_init(text, mask_x1 - mask_x0, scale);

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
			if(CAT_break_list_lookup(glyph_idx))
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

		CAT_set_draw_colour(colour);
		CAT_set_draw_scale(scale);
		//CAT_set_draw_mask(mask_x0, mask_y0, mask_x1, mask_y1);
		CAT_draw_sprite(&glyph_sprite, *glyph_ptr, cursor_x, cursor_y);
		cursor_x += CAT_GLYPH_WIDTH * scale;
		glyph_ptr++; glyph_idx++;
	}

	return cursor_y;
}

int CAT_draw_textf(int x, int y, const char* fmt, ...)
{
	static char buf[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 512, fmt, args);
	va_end(args);

	return CAT_draw_text(x, y, buf);
}