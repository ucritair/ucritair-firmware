#include "cat_render.h"

#include "sprite_assets.h"
#include <math.h>
#include <cat_math.h>

//////////////////////////////////////////////////////////////////////////
// SPRITER

static CAT_draw_flag draw_flags = CAT_DRAW_FLAG_NONE;
static uint16_t draw_colour = CAT_TRANSPARENT;
static uint8_t draw_scale = 1;
static CAT_rect draw_mask = {{-1, -1}, {-1, -1}};

void CAT_set_draw_flags(int flags)
{
	draw_flags = flags;
}

CAT_draw_flag consume_draw_flags()
{
	CAT_draw_flag value = draw_flags;
	draw_flags = CAT_DRAW_FLAG_NONE;
	return value;
}

void CAT_set_draw_colour(uint16_t colour)
{
	draw_colour = colour;
}

uint16_t consume_draw_colour()
{
	uint16_t value = draw_colour;
	draw_colour = CAT_TRANSPARENT;
	return value;
}

void CAT_set_draw_scale(uint8_t scale)
{
	draw_scale = scale;
}

uint8_t consume_draw_scale()
{
	uint8_t value = draw_scale;
	draw_scale = 1;
	return value;
}

void CAT_set_draw_mask(int x0, int y0, int x1, int y1)
{
	draw_mask = (CAT_rect){{x0, y0}, {x1, y1}};
}

CAT_rect consume_draw_mask()
{
	CAT_rect value = draw_mask;
	draw_mask = (CAT_rect){{-1, -1}, {-1, -1}};
	return value;
}

static inline bool rect_clip(int x0, int y0, int x1, int y1, int x, int y)
{
	if(x < x0 || x >= x1)
		return false;
	if(y < y0 || y >= y1)
		return false;
	return true;
}

void CAT_draw_sprite(const CAT_sprite* sprite, int frame_idx, int x, int y)
{
	if(sprite == NULL)
	{
		CAT_printf("[WARNING] CAT_draw_sprite: null sprite\n");
		sprite = &null_sprite;
	}
	if(frame_idx == -1)
		frame_idx = CAT_animator_get_frame(sprite);
	if(frame_idx < 0 || frame_idx >= sprite->frame_count)
	{
		CAT_printf("[WARNING] CAT_draw_sprite: frame index %d out of bounds\n", frame_idx);
		frame_idx = sprite->frame_count-1;
	}

	uint16_t* framebuffer = CAT_LCD_get_framebuffer();
	CAT_draw_flag flags = consume_draw_flags();
	uint16_t colour = consume_draw_colour();
	uint8_t scale = consume_draw_scale();	
	CAT_rect mask = consume_draw_mask();

	uint16_t w = sprite->width * scale;
	uint16_t h = sprite->height * scale;
	if ((flags & CAT_DRAW_FLAG_CENTER_X) > 0)
			x -= w / 2;
	if ((flags & CAT_DRAW_FLAG_CENTER_Y) > 0)
		y -= h / 2;
	else if ((flags & CAT_DRAW_FLAG_BOTTOM) > 0)
		y -= h;
	bool reflect_x = (flags & CAT_DRAW_FLAG_REFLECT_X) > 0;
	
	y -= FRAMEBUFFER_ROW_OFFSET;
	if (y >= CAT_LCD_FRAMEBUFFER_H)
		return;
	int y_f = y + h;
	if (y_f < 0)
		return;
	if (y_f > CAT_LCD_FRAMEBUFFER_H+scale)
		y_f = CAT_LCD_FRAMEBUFFER_H+scale;
	
	if(x >= CAT_LCD_FRAMEBUFFER_W)
		return;
	int x_f = x + w;
	if(x_f < 0)
		return;
	
	int mask_x0 = mask.min.x == -1 ? x : mask.min.x;
	int mask_y0 = mask.min.y == -1 ? y : mask.min.y - FRAMEBUFFER_ROW_OFFSET;
	int mask_x1 = mask.max.x == -1 ? x_f : mask.max.x;
	int mask_y1 = mask.max.y == -1 ? y_f : mask.max.y - FRAMEBUFFER_ROW_OFFSET;
	mask_x0 = max(mask_x0, 0);
	mask_y0 = max(mask_y0, 0);
	mask_x1 = min(mask_x1, CAT_LCD_FRAMEBUFFER_W);
	mask_y1 = min(mask_y1, CAT_LCD_FRAMEBUFFER_H);

	const uint8_t* frame = sprite->frames[frame_idx];
	uint16_t run_idx = 0;

	uint8_t token;
	uint8_t c_idx; uint16_t c;
	uint8_t run_length; uint8_t run_remainder;

	int y_w;
	int x_start; int x_end; int dx;
	int x_w;

	if(scale > 1)
	{
		y_w = y;
		x_start = reflect_x ? x_f-scale : x;
		x_end = reflect_x ? x-scale : x_f;
		dx = reflect_x ? -1 : 1;
		x_w = x_start;

		while(y_w < y_f)
		{
			token = frame[run_idx++];

			c_idx = token == 0xff ? frame[run_idx++] : token;
			c = sprite->colour_table[c_idx];
			run_length = token == 0xff ? frame[run_idx++] : 1;
			
			if(c == CAT_TRANSPARENT)
			{
				int width = sprite->width;
				int run_rows = (run_length / width) * scale;
				int run_off = (run_length % width) * scale;
				x_w += run_off * dx;
				int overshoot = x_w - x_end;
				if((overshoot*dx) >= 0)
				{
					y_w += scale;
					x_w = x_start + overshoot;
				}
				y_w += run_rows;
				if(y_w > y_f)
					return;
				continue;
			}

			c = colour ==
			CAT_TRANSPARENT ?
			ADAPT_EMBEDDED_COLOUR(c) :
			ADAPT_DESKTOP_COLOUR(colour);
			run_remainder = run_length;
	
			while(run_remainder > 0)
			{
				for(int i = 0; i < scale && y_w < y_f; i++)
				{
					for(int j = 0; j < scale; j++)
					{
						if(rect_clip(mask_x0, mask_y0, mask_x1, mask_y1, x_w, y_w))
							framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x_w] = c;
						x_w += dx;
					}
					y_w += 1;
					if(y_w > y_f)
						return;
					x_w -= dx * scale;
				}
				y_w -= scale;
					
				x_w += dx * scale;
				if (x_w == x_end)
				{
					y_w += scale;
					x_w = x_start;
				}
				run_remainder -= 1;
			}
		}
	}
	else
	{
		y_w = y;
		x_start = reflect_x ? x_f-1 : x;
		x_end = reflect_x ? x-1 : x_f;
		dx = reflect_x ? -1 : 1;
		x_w = x_start;

		while(y_w < y_f)
		{
			token = frame[run_idx++];

			c_idx = token == 0xff ? frame[run_idx++] : token;
			c = sprite->colour_table[c_idx];
			run_length = token == 0xff ? frame[run_idx++] : 1;

			if(c == CAT_TRANSPARENT)
			{
				int width = sprite->width;
				int run_rows = run_length / width;
				int run_off = run_length % width;
				x_w += run_off * dx;
				int overshoot = x_w - x_end;
				if((overshoot*dx) >= 0)
				{
					y_w += scale;
					x_w = x_start + overshoot;
				}
				y_w += run_rows;
				if(y_w > y_f)
					return;
				continue;
			}

			c = colour ==
			CAT_TRANSPARENT ?
			ADAPT_EMBEDDED_COLOUR(c) :
			ADAPT_DESKTOP_COLOUR(colour);
			run_remainder = run_length;
			
			while(run_remainder > 0)
			{
				if(rect_clip(mask_x0, mask_y0, mask_x1, mask_y1, x_w, y_w))
					framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x_w] = c;

				x_w += dx;
				if (x_w == x_end)
				{
					x_w = x_start;
					y_w += 1;
					if(y_w >= y_f)
						return;
				}
				run_remainder -= 1;
			}
		}
	}
}