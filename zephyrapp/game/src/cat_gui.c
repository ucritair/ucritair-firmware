#include "cat_gui.h"

#include "cat_sprite.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "cat_machine.h"
#include <math.h>
#include <ctype.h>
#include "cat_input.h"

//////////////////////////////////////////////////////////////////////////
// RENDERING

CAT_gui gui =
{
	.flags = CAT_GUI_DEFAULT,

	.start = (CAT_ivec2) {0, 0},
	.shape = (CAT_ivec2) {0, 0},
	.cursor = (CAT_ivec2) {0, 0},

	.margin = 8,
	.pad = 4,
	.channel_height = 0
};

void CAT_gui_set_flag(CAT_gui_flag flag)
{
	gui.flags |= (1 << flag);
}

bool CAT_gui_consume_flag(CAT_gui_flag flag)
{
	bool value = (gui.flags & (1 << flag)) > 0;
	gui.flags &= ~(1 << flag);
	return value;
}

void CAT_gui_panel(CAT_ivec2 start, CAT_ivec2 shape)
{
	spriter.mode = CAT_DRAW_MODE_DEFAULT;
	CAT_fillberry(start.x * CAT_TILE_SIZE, start.y * CAT_TILE_SIZE, shape.x * CAT_TILE_SIZE, shape.y * CAT_TILE_SIZE, 0xFFFF);
	if(!CAT_gui_consume_flag(CAT_GUI_NO_BORDER))
		CAT_strokeberry(start.x * CAT_TILE_SIZE, start.y * CAT_TILE_SIZE, shape.x * CAT_TILE_SIZE, shape.y * CAT_TILE_SIZE, 0x0000);

	gui.start = start;
	gui.shape = shape;
	gui.cursor = CAT_ivec2_mul(start, CAT_TILE_SIZE);
	gui.cursor.y += gui.margin;
	gui.cursor.x += gui.margin;
	if(CAT_gui_consume_flag(CAT_GUI_TIGHT))
	{
		gui.cursor.y -= gui.margin/2;
		gui.cursor.x -= gui.margin/2;
	}
	gui.channel_height = 0;
}

void gui_open_channel(int height)
{
	if(gui.channel_height == 0)
		gui.cursor.y += height / 2;
	if(height > gui.channel_height)
		gui.channel_height = height;
}

void CAT_gui_line_break()
{
	gui.cursor.y +=
	(gui.channel_height > 0 ?
	gui.channel_height / 2 :
	CAT_GLYPH_HEIGHT) +
	gui.pad;
	gui.cursor.x = gui.start.x * CAT_TILE_SIZE + gui.margin;
	gui.channel_height = 0;
}

void CAT_gui_text(const char* text)
{
	spriter.mode = CAT_DRAW_MODE_CENTER_Y;

	bool wrap = CAT_gui_consume_flag(CAT_GUI_WRAP_TEXT);
	int x_lim = (gui.start.x * CAT_TILE_SIZE) + (gui.shape.x) * CAT_TILE_SIZE - CAT_GLYPH_WIDTH - gui.margin;
	const char* c = text;

	while(*c != '\0')
	{
		if(wrap && gui.cursor.x >= x_lim && !isspace(*(c+1)))
		{
			if(!isspace(*c) && !isspace(*(c-1)))
				CAT_draw_sprite(&glyph_sprite, '-', gui.cursor.x, gui.cursor.y);
			CAT_gui_line_break();	
			if(isspace(*c))
				c++;
		}

		if(*c == '\n')
		{
			CAT_gui_line_break();
			c++;
			continue;
		}
		if(*c == '\t')
		{
			gui.cursor.x += CAT_GLYPH_WIDTH * 4;
			c++;
			continue;
		}

		gui_open_channel(CAT_GLYPH_HEIGHT);
		CAT_draw_sprite(&glyph_sprite, *c, gui.cursor.x, gui.cursor.y);
		gui.cursor.x += CAT_GLYPH_WIDTH;
		c++;
	}
}

void CAT_gui_image(CAT_sprite* sprite, int frame_idx)
{
	spriter.mode = CAT_DRAW_MODE_CENTER_Y;

	gui_open_channel(sprite->height);

	gui.cursor.x += gui.pad / 2;
	CAT_draw_sprite(sprite, frame_idx, gui.cursor.x, gui.cursor.y);
	gui.cursor.x += sprite->width;
	gui.cursor.x += gui.pad / 2;
}

void CAT_gui_div(const char* text)
{
	spriter.mode = CAT_DRAW_MODE_CENTER_Y;
	
	CAT_gui_line_break();
	gui_open_channel(CAT_TILE_SIZE);
	if(strlen(text) == 0)
	{
		CAT_lineberry(0, gui.cursor.y, LCD_SCREEN_W, gui.cursor.y, 0x0000);
	}
	else
	{
		CAT_gui_text(text);
		CAT_lineberry(gui.cursor.x + gui.pad, gui.cursor.y, LCD_SCREEN_W-gui.margin, gui.cursor.y, 0x0000);
	}
	CAT_gui_line_break();
}

void CAT_gui_textf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char text[256];
	vsprintf(text, fmt, args);
	va_end(args);
	CAT_gui_text(text);
}

void CAT_gui_title(bool tabs, CAT_sprite* a_action, CAT_sprite* b_action, const char* fmt, ...)
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	
	if(tabs)
		CAT_gui_text("< ");
	va_list args;
	va_start(args, fmt);
	char text[256];
	vsprintf(text, fmt, args);
	va_end(args);
	CAT_gui_text(text);
	if(tabs)
		CAT_gui_text(" >");
	CAT_gui_text(" ");

	if(a_action != NULL)
	{
		CAT_gui_image(&icon_a_sprite, 1);
		CAT_gui_image(a_action, 0);
	}
	if(b_action != NULL)
	{
		CAT_gui_image(&icon_b_sprite, 1);
		CAT_gui_image(b_action, 0);
	}
}

static const char* cases[] = 
{
	"0123456789\x06\nQWERTYUIOP\nASDFGHJKL\x07\x09\nZXCVBNM\x08\n",
	"0123456789\x06\nqwertyuiop\nasdfghjkl\x07\x09\nzxcvbnm\x08\n"
};

struct 
{
	char buffer[64];
	int cursor;
	bool finalized;
	
	bool show_cursor;
	float cursor_timer;

	int case_idx;
} keyboard = 
{
	.cursor = 0,
	.finalized = false,
	.show_cursor = true,
	.cursor_timer = 0.0f,
	.case_idx = 0
};
bool keyboard_open = false;

bool CAT_gui_keyboard()
{
	keyboard.cursor_timer += CAT_get_delta_time();
	if(keyboard.cursor_timer >= 0.5f)
	{
		keyboard.cursor_timer = 0.0f;
		keyboard.show_cursor = !keyboard.show_cursor;
	}
		
	CAT_gui_panel((CAT_ivec2){0, 10}, (CAT_ivec2){15, 10});
	CAT_gui_text(keyboard.buffer);
	if(keyboard.show_cursor)
		CAT_gui_text("|");
	gui.cursor.y -= 4;
	CAT_gui_div("");

	spriter.mode = CAT_DRAW_MODE_DEFAULT;
	int x_w = gui.margin * 2;
	int y_w = gui.cursor.y;
	const char* c = cases[keyboard.case_idx];
	while(*c != '\0')
	{
		if(*c == '\n')
		{
			y_w += CAT_GLYPH_HEIGHT + 8;
			x_w = gui.margin * 2;
			c++;
			continue;
		}
		
		CAT_strokeberry(x_w, y_w, CAT_GLYPH_WIDTH + 4, CAT_GLYPH_HEIGHT + 6, 0);
		CAT_draw_sprite(&glyph_sprite, *c, x_w + 2, y_w + 3);
		if(CAT_input_touch_rect(x_w, y_w, CAT_GLYPH_WIDTH + 4, CAT_GLYPH_HEIGHT + 6))
		{
			if(*c == 6)
			{
				if(keyboard.cursor > 0)
				{
					keyboard.cursor -= 1;
					keyboard.show_cursor = true;
					keyboard.cursor_timer = 0;
				}
			}
			else if(*c == 7)
			{
				keyboard.finalized = true;
				keyboard_open = false;
			}
			else if(*c == 8)
				keyboard.case_idx = !keyboard.case_idx;
			else if(*c == 9)
			{
				keyboard.cursor = 0;
				keyboard_open = false;
			}
			else
			{
				keyboard.buffer[keyboard.cursor] = *c;
				keyboard.cursor += 1;
			}	
		}
		keyboard.buffer[keyboard.cursor] = '\0';

		x_w += CAT_GLYPH_WIDTH + 8;
		c++;
	}

	return keyboard_open;
}

bool CAT_gui_harvest_keyboard(char* destination)
{
	if(keyboard.finalized)
	{
		if(destination != NULL)
			memcpy(destination, keyboard.buffer, keyboard.cursor+1);
		keyboard.cursor = 0;
		keyboard.finalized = false;
		keyboard.case_idx = 0;
		return true;
	}
	return false;
}