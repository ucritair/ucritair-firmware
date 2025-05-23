#include "cat_render.h"

#include "sprite_assets.h"
#include <math.h>
#include <cat_math.h>

//////////////////////////////////////////////////////////////////////////
// SPRITER

static CAT_draw_flag draw_flags = CAT_DRAW_FLAG_DEFAULT;
static uint16_t draw_colour = CAT_TRANSPARENT;
static uint8_t draw_scale = 1;

void CAT_push_draw_flags(int flags)
{
	draw_flags = flags;
}

CAT_draw_flag consume_draw_flags()
{
	CAT_draw_flag value = draw_flags;
	draw_flags = CAT_DRAW_FLAG_DEFAULT;
	return value;
}

void CAT_push_draw_colour(uint16_t colour)
{
	draw_colour = colour;
}

uint16_t consume_draw_colour()
{
	uint16_t value = draw_colour;
	draw_colour = CAT_TRANSPARENT;
	return value;
}

void CAT_push_draw_scale(uint8_t scale)
{
	draw_scale = scale;
}

uint8_t consume_draw_scale()
{
	uint8_t value = draw_scale;
	draw_scale = 1;
	return value;
}

void CAT_draw_sprite(const CAT_sprite* sprite, int16_t frame_idx, int16_t x, int16_t y)
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
	CAT_draw_flag flags_override = consume_draw_flags();
	uint16_t colour_override = consume_draw_colour();
	uint8_t scale_override = consume_draw_scale();	

	uint16_t w = sprite->width * scale_override;
	uint16_t h = sprite->height * scale_override;
	if ((flags_override & CAT_DRAW_FLAG_CENTER_X) > 0)
			x -= w / 2;
	if ((flags_override & CAT_DRAW_FLAG_CENTER_Y) > 0)
		y -= h / 2;
	else if ((flags_override & CAT_DRAW_FLAG_BOTTOM) > 0)
		y -= h;
	bool reflect_x = (flags_override & CAT_DRAW_FLAG_REFLECT_X) > 0;
	
	y -= FRAMEBUFFER_ROW_OFFSET;
	if (y >= CAT_LCD_FRAMEBUFFER_H)
		return;
	int16_t y_f = y + h;
	if (y_f < 0)
		return;
	if (y_f > CAT_LCD_FRAMEBUFFER_H)
		y_f = CAT_LCD_FRAMEBUFFER_H-1;
	
	if(x >= CAT_LCD_FRAMEBUFFER_W)
		return;
	int16_t x_f = x + w;
	if(x_f < 0)
		return;

	const uint8_t* frame = sprite->frames[frame_idx];
	uint16_t run_idx = 0;

	if(scale_override != 1)
	{
		int16_t y_w = y;
		int16_t x_start = reflect_x ? x_f-scale_override : x;
		int16_t x_end = reflect_x ? x-scale_override : x_f;
		int16_t dx = reflect_x ? -1 : 1;
		int16_t x_w = x_start;

		while(y_w < y_f)
		{
			uint8_t token = frame[run_idx++];

			uint16_t colour_idx = token == 0xff ? frame[run_idx++] : token;
			uint16_t colour = sprite->colour_table[colour_idx];
			bool transparent = colour == CAT_TRANSPARENT;
			colour = colour_override == CAT_TRANSPARENT ?
			ADAPT_EMBEDDED_COLOUR(colour) : ADAPT_DESKTOP_COLOUR(colour_override);

			uint8_t run_length = token == 0xff ? frame[run_idx++] : 1;
			uint8_t run_remainder = run_length;
			
			while(run_remainder > 0)
			{
				for(int i = 0; i < scale_override && y_w < y_f; i++)
				{
					for(int j = 0; j < scale_override; j++)
					{
						if
						(
							!transparent &&
							y_w >= 0 && y_w < CAT_LCD_FRAMEBUFFER_H &&
							x_w >= 0 && x_w < CAT_LCD_FRAMEBUFFER_W
						)
						{
							framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x_w] = colour;
						}
						x_w += dx;
					}
					y_w += 1;
					x_w -= dx * scale_override;
				}
				y_w -= scale_override;
					
				x_w += dx * scale_override;
				if (x_w == x_end)
				{
					y_w += scale_override;
					x_w = x_start;
				}
				run_remainder -= 1;
			}
		}
	}
	else
	{
		int16_t y_w = y;
		int16_t x_start = reflect_x ? x_f-1 : x;
		int16_t x_end = reflect_x ? x-1 : x_f;
		int16_t dx = reflect_x ? -1 : 1;
		int16_t x_w = x_start;

		while(y_w < y_f)
		{
			uint8_t token = frame[run_idx++];

			uint16_t colour_idx = token == 0xff ? frame[run_idx++] : token;
			uint16_t colour = sprite->colour_table[colour_idx];
			bool transparent = colour == CAT_TRANSPARENT;
			colour = colour_override == CAT_TRANSPARENT ?
			ADAPT_EMBEDDED_COLOUR(colour) : ADAPT_DESKTOP_COLOUR(colour_override);

			uint8_t run_length = token == 0xff ? frame[run_idx++] : 1;
			uint8_t run_remainder = run_length;
			
			while(run_remainder > 0)
			{
				if
				(
					!transparent &&
					y_w >= 0 && y_w < CAT_LCD_FRAMEBUFFER_H &&
					x_w >= 0 && x_w < CAT_LCD_FRAMEBUFFER_W
				)
				{
					framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x_w] = colour;
				}

				x_w += dx;
				if (x_w == x_end)
				{
					x_w = x_start;
					y_w += 1;
				}
				run_remainder -= 1;
			}
		}
	}
}