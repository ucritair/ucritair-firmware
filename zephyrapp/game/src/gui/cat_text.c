#include "cat_gui.h"

#include "sprite_assets.h"
#include <stdarg.h>
#include <stdio.h>
#include "cat_text.h"

//////////////////////////////////////////////////////////////////////////
// WRAPPING

int break_list[CAT_TEXT_MAX_LINES];
int break_count = 0;

// Word Length
int wrdlen(const char* txt, int idx)
{
	int length = 0;
	for(int i = idx; txt[i] != ' ' && txt[i] != '\n' && txt[i] != '\0'; i++)
	{
		length += 1;
	}
	return length;
}

// Line Visual Length
int lnvlen(const char* txt, int idx)
{
	int left = idx;
	for(; txt[left] != '\n' && txt[left] != '\0'; left++)
	{
		if(txt[left] != ' ')
			break;
	}
	int right = left;
	for(; txt[right] != '\n' && txt[right] != '\0'; right++);
	if(txt[right] == ' ')
		right -= 1;

	bool inside = false;
	int hidden = 0;
	for(int i = left; i < right; i++)
	{
		if(!inside)
		{
			if(!strncmp(txt+i, "<c", 2))
			{
				inside = true;
				hidden += 1;
			}
			if(!strncmp(txt+i, "</c>", 4))
			{
				hidden += 4;
				i += 5;
			}
		}
		else
		{
			hidden += 1;
			if(!strncmp(txt+i, ">", 1))
				inside = false;
		}
	}
	return right - left - hidden;
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

		if ((word_width + space_width) > slack && idx != 0)
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

int CAT_break_list_lookup(int idx)
{
	for(int i = 0; i < break_count; i++)
	{
		if(break_list[i] == idx)
			return i;
	}
	return -1;
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

int get_centered_x(int x, const char* text, int glyph_idx, int scale)
{
	if(break_count > 0)
	{
		int break_idx = -1;
		for(int i = 0; i < break_count; i++)
		{
			if(break_list[i] >= glyph_idx)
			{
				break_idx = i;
				break;
			}
		}
		int start = glyph_idx;
		int end = break_idx == -1 ? strlen(text) : break_list[break_idx];
		if(text[end-1] == ' ')
			end -= 1;

		int l = end - start;
		int w = l * CAT_GLYPH_WIDTH * scale;
		return x - w / 2;
	}
	else
	{
		int l = lnvlen(text, glyph_idx);
		int w = l * CAT_GLYPH_WIDTH * scale;
		return x - w / 2;
	}
}

int CAT_draw_text(int x, int y, const char* text)
{
	int scale = consume_text_scale();
	uint16_t colour = consume_text_colour();

	int flags = consume_text_flags();
	bool wrap = flags & CAT_TEXT_FLAG_WRAP;
	bool center = flags & CAT_TEXT_FLAG_CENTER;
	bool vertical = flags & CAT_TEXT_FLAG_VERTICAL;

	CAT_rect mask = consume_text_mask();
	if(center)
	{
		mask.min.x = x-CAT_LCD_SCREEN_W;
		mask.max.x = x+CAT_LCD_SCREEN_W;
	}
	else
	{
		mask.min.x = x;
		if(!wrap)
			mask.max.x = CAT_INT_MAX;
	}
	mask.min.y = y;
	mask.max.y = y;

	CAT_reset_text_box();
	CAT_set_text_box(mask.min.x, mask.min.y, mask.max.x, mask.max.y);
	if(vertical)
		CAT_set_text_box_alignment(CAT_TEXT_ALIGNMENT_VERTICAL);
	else if(center)
		CAT_set_text_box_alignment(CAT_TEXT_ALIGNMENT_CENTER);
	else
		CAT_set_text_box_alignment(CAT_TEXT_ALIGNMENT_LEFT);
	CAT_text_box_draw(scale, colour, text);

	return CAT_get_text_box_cursor_y();
}

static size_t last_strlen = 0;

int CAT_draw_textf(int x, int y, const char* fmt, ...)
{
	static char buffer[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	last_strlen = strlen(buffer);
	return CAT_draw_text(x, y, buffer);
}

size_t CAT_get_drawn_strlen()
{
	return last_strlen;
}

const char* CAT_fmt_float(float f)
{
	static char buf[128];
	static int ptr = 0;

	if(CAT_is_first_render_cycle())
		ptr = 0;

	int whole = (int) f;
	float frac = f - whole;
	unsigned fracu = (unsigned) (frac * 100);
	ptr += snprintf(buf+ptr, sizeof(buf)-ptr, "%d.%2.2u", whole, fracu);
	return buf+ptr;
}