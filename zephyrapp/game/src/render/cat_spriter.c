#include "cat_render.h"

#include "sprite_assets.h"
#include <math.h>
#include <cat_math.h>
#include "cat_core.h"

//////////////////////////////////////////////////////////////////////////
// SPRITER

static uint64_t draw_flags = CAT_DRAW_FLAG_NONE;
static uint16_t draw_colour;
static uint8_t draw_scale;
static int draw_mask_x0;
static int draw_mask_y0;
static int draw_mask_x1;
static int draw_mask_y1;

static uint64_t sprite_overrides = CAT_SPRITE_OVERRIDE_NONE;

void CAT_set_sprite_flags(uint64_t flags)
{
	draw_flags = flags;
	sprite_overrides |= CAT_SPRITE_OVERRIDE_FLAGS;
}

void CAT_set_sprite_colour(uint16_t colour)
{
	draw_colour = CAT_ADAPT_DESKTOP_COLOUR(colour);
	sprite_overrides |= CAT_SPRITE_OVERRIDE_COLOUR;
}

void CAT_set_sprite_scale(uint8_t scale)
{
	draw_scale = scale;
	if(scale != 1)
		sprite_overrides |= CAT_SPRITE_OVERRIDE_SCALE;
}

void CAT_set_sprite_mask(int x0, int y0, int x1, int y1)
{
	draw_mask_x0 = x0;
	draw_mask_y0 = y0;
	draw_mask_x1 = x1;
	draw_mask_y1 = y1;
	sprite_overrides |= CAT_SPRITE_OVERRIDE_MASK;
}

static inline bool rect_clip(int x0, int y0, int x1, int y1, int x, int y)
{
	return
	x >= x0 &&
	x < x1 &&
	y >= y0 &&
	y < y1;
}

void CAT_draw_sprite(const CAT_sprite* sprite, int frame_idx, int x, int y)
{
	if(sprite == NULL)
	{
		CAT_printf("[WARNING] CAT_draw_sprite: null sprite\n");
		sprite = &null_sprite;
	}

	frame_idx = frame_idx == -1 ? CAT_animator_get_frame(sprite) : frame_idx;
	if(frame_idx < 0 || frame_idx >= sprite->frame_count)
	{
		CAT_printf("[WARNING] CAT_draw_sprite: frame index %d out of bounds\n", frame_idx);
		frame_idx = sprite->frame_count-1;
	}

	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	uint16_t w = sprite->width * draw_scale;
	uint16_t h = sprite->height * draw_scale;
	if (draw_flags & CAT_DRAW_FLAG_CENTER_X)
		x -= w / 2;
	if (draw_flags & CAT_DRAW_FLAG_CENTER_Y)
		y -= h / 2;
	else if (draw_flags & CAT_DRAW_FLAG_BOTTOM)
		y -= h;
	
	y -= CAT_LCD_FRAMEBUFFER_OFFSET;
	if (y >= CAT_LCD_FRAMEBUFFER_H)
		goto draw_sprite_exit;	
	int y_f = y + h;
	if (y_f < 0)
		goto draw_sprite_exit;	
	if (y_f > CAT_LCD_FRAMEBUFFER_H+draw_scale)
		y_f = CAT_LCD_FRAMEBUFFER_H+draw_scale;
	if(x >= CAT_LCD_FRAMEBUFFER_W)
		goto draw_sprite_exit;	
	int x_f = x + w;
	if(x_f < 0)
		goto draw_sprite_exit;	
	
	draw_mask_x0 = CAT_max(draw_mask_x0, 0);
	draw_mask_y0 = CAT_max(draw_mask_y0-CAT_LCD_FRAMEBUFFER_OFFSET, 0);
	draw_mask_x1 = CAT_min(draw_mask_x1, CAT_LCD_FRAMEBUFFER_W);
	draw_mask_y1 = CAT_min(draw_mask_y1-CAT_LCD_FRAMEBUFFER_OFFSET, CAT_LCD_FRAMEBUFFER_H);

	const uint8_t* frame = sprite->frames[frame_idx];
	uint16_t run_idx = 0;

	uint8_t token;
	uint8_t c_idx; uint16_t c;
	uint8_t run_length, run_remainder;

	int x_start, x_end, dx;
	int x_w, y_w;

	if(sprite_overrides & CAT_SPRITE_OVERRIDE_SCALE)
	{
		y_w = y;
		x_start = draw_flags & CAT_DRAW_FLAG_REFLECT_X ? x_f-draw_scale : x;
		x_end = draw_flags & CAT_DRAW_FLAG_REFLECT_X ? x-draw_scale : x_f;
		dx = draw_flags & CAT_DRAW_FLAG_REFLECT_X ? -1 : 1;
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
				int run_rows = (run_length / width) * draw_scale;
				int run_off = (run_length % width) * draw_scale;
				x_w += run_off * dx;
				int overshoot = x_w - x_end;
				if((overshoot*dx) >= 0)
				{
					y_w += draw_scale;
					x_w = x_start + overshoot;
				}
				y_w += run_rows;
				if(y_w > y_f)
					goto draw_sprite_exit;
				continue;
			}

			if(sprite_overrides & CAT_SPRITE_OVERRIDE_COLOUR)
				c = draw_colour;
			else
				c = CAT_ADAPT_EMBEDDED_COLOUR(c);
			run_remainder = run_length;
	
			while(run_remainder > 0)
			{
				for(int i = 0; i < draw_scale && y_w < y_f; i++)
				{
					for(int j = 0; j < draw_scale; j++)
					{
						if(rect_clip(draw_mask_x0, draw_mask_y0, draw_mask_x1, draw_mask_y1, x_w, y_w))
							framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x_w] = c;
						x_w += dx;
					}
					y_w += 1;
					if(y_w > y_f)
						goto draw_sprite_exit;
					x_w -= dx * draw_scale;
				}
				y_w -= draw_scale;
					
				x_w += dx * draw_scale;
				if (x_w == x_end)
				{
					y_w += draw_scale;
					x_w = x_start;
				}
				run_remainder -= 1;
			}
		}
	}
	else
	{
		y_w = y;
		x_start = draw_flags & CAT_DRAW_FLAG_REFLECT_X ? x_f-1 : x;
		x_end = draw_flags & CAT_DRAW_FLAG_REFLECT_X ? x-1 : x_f;
		dx = draw_flags & CAT_DRAW_FLAG_REFLECT_X ? -1 : 1;
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
					y_w += draw_scale;
					x_w = x_start + overshoot;
				}
				y_w += run_rows;
				if(y_w > y_f)
					return;
				continue;
			}

			if(sprite_overrides & CAT_SPRITE_OVERRIDE_COLOUR)
				c = draw_colour;
			else
				c = CAT_ADAPT_EMBEDDED_COLOUR(c);
			run_remainder = run_length;
			
			while(run_remainder > 0)
			{
				if(rect_clip(draw_mask_x0, draw_mask_y0, draw_mask_x1, draw_mask_y1, x_w, y_w))
					framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x_w] = c;

				x_w += dx;
				if (x_w == x_end)
				{
					x_w = x_start;
					y_w += 1;
					if(y_w >= y_f)
						goto draw_sprite_exit;
				}
				run_remainder -= 1;
			}
		}
	}

draw_sprite_exit:
	draw_flags = CAT_DRAW_FLAG_NONE;
	draw_colour = CAT_TRANSPARENT;
	draw_scale = 1;
	draw_mask_x0 = 0;
	draw_mask_y0 = 0;
	draw_mask_x1 = CAT_LCD_SCREEN_W;
	draw_mask_y1 = CAT_LCD_SCREEN_H;
	sprite_overrides = CAT_SPRITE_OVERRIDE_NONE;
}

void CAT_draw_sprite_raw(const CAT_sprite* sprite, int frame_idx, int x, int y)
{
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();
	frame_idx = frame_idx == -1 ? CAT_animator_get_frame(sprite) : frame_idx;

	uint16_t w = sprite->width;
	uint16_t h = sprite->height;
	if (draw_flags & CAT_DRAW_FLAG_CENTER_X)
		x -= w / 2;
	if (draw_flags & CAT_DRAW_FLAG_CENTER_Y)
		y -= h / 2;
	else if (draw_flags & CAT_DRAW_FLAG_BOTTOM)
		y -= h;

	y -= CAT_LCD_FRAMEBUFFER_OFFSET;
	if(y >= CAT_LCD_FRAMEBUFFER_H)
		goto draw_sprite_raw_exit;
	if((y + h) < 0)
		goto draw_sprite_raw_exit;
	
	uint16_t clipped_rows = y < draw_mask_y0 ? draw_mask_y0-y : 0;
	h = CAT_min(h, CAT_min(CAT_LCD_FRAMEBUFFER_H, draw_mask_y1)-y);
	if(h <= clipped_rows)
		goto draw_sprite_raw_exit;
	uint16_t pitch = CAT_LCD_FRAMEBUFFER_W-w;

	const uint8_t* frame = sprite->frames[frame_idx];
	uint16_t run_idx = 0;
	uint16_t dx = 0;
	uint16_t dy = 0;
	uint16_t px_idx = y*CAT_LCD_FRAMEBUFFER_W+x;

	while(dy < h)
	{
		uint8_t token = frame[run_idx++];
		uint8_t c_idx = token == 0xff ? frame[run_idx++] : token;
		uint16_t c = sprite->colour_table[c_idx];
		uint16_t run_length = token == 0xff ? frame[run_idx++] : 1;
		uint16_t run_remainder = run_length;

		if(c == CAT_TRANSPARENT)
		{
			uint16_t run = dx + run_length;
			uint8_t run_rows = run / w;
			uint8_t run_off = run % w;
			dy += run_rows;
			dx = run_off;
			px_idx += (run_rows * pitch) + run_length;
			continue;
		}
		if(sprite_overrides & CAT_SPRITE_OVERRIDE_COLOUR)
			c = draw_colour;
		else
			c = CAT_ADAPT_EMBEDDED_COLOUR(c);
		
		while(run_remainder > 0 && dy < h)
		{
			if(dy >= clipped_rows && x+dx >= draw_mask_x0 && x+dx < draw_mask_x1)
				framebuffer[px_idx] = c;
			
			dx += 1;
			px_idx += 1;
			if (dx >= w)
			{
				dx = 0;
				dy += 1;
				px_idx += pitch;
			}
			run_remainder -= 1;
		}
	}

draw_sprite_raw_exit:
	draw_flags = CAT_DRAW_FLAG_NONE;
	draw_colour = CAT_TRANSPARENT;
	draw_scale = 1;
	draw_mask_x0 = 0;
	draw_mask_y0 = 0;
	draw_mask_x1 = CAT_LCD_SCREEN_W;
	draw_mask_y1 = CAT_LCD_SCREEN_H;
	sprite_overrides = CAT_SPRITE_OVERRIDE_NONE;
}

void CAT_draw_background(const CAT_sprite* sprite, int frame_idx, int y)
{
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	y -= CAT_LCD_FRAMEBUFFER_OFFSET;
	if (y >= CAT_LCD_FRAMEBUFFER_H)
		return;
	
	const uint8_t* frame = sprite->frames[frame_idx];
	uint16_t run_idx = 0;
	uint16_t px_idx = y*CAT_LCD_FRAMEBUFFER_W;
	uint16_t dx = 0;
	uint16_t dy = 0;

	uint16_t w = sprite->width;
	uint16_t h = sprite->height;
	uint16_t clipped_rows = CAT_max(-y, 0);
	h = CAT_min(h, CAT_LCD_FRAMEBUFFER_H-y);
	if(h <= clipped_rows)
		return;
	uint16_t pitch = CAT_LCD_FRAMEBUFFER_W-w;

	while(dy < h)
	{
		uint8_t token = frame[run_idx++];
		uint8_t c_idx = token == 0xff ? frame[run_idx++] : token;
		uint16_t c = sprite->colour_table[c_idx];
		uint16_t run_length = token == 0xff ? frame[run_idx++] : 1;
		uint16_t run_remainder = run_length;

		if(c == CAT_TRANSPARENT)
		{
			uint16_t run = dx + run_length;
			uint8_t run_rows = run / w;
			uint8_t run_off = run % w;
			dy += run_rows;
			dx = run_off;
			px_idx += run_length;
			continue;
		}

		c = CAT_ADAPT_EMBEDDED_COLOUR(c);
		
		while(run_remainder > 0 && dy < h)
		{
			if(dy >= clipped_rows)
				framebuffer[px_idx] = c;
			px_idx += 1;

			dx += 1;
			if (dx >= w)
			{
				dx = 0;
				dy += 1;
				px_idx += pitch;
			}
			run_remainder -= 1;
		}
	}
}

void CAT_draw_tile(const CAT_sprite* sprite, int frame_idx, int x, int y)
{
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();
	
	y -= CAT_LCD_FRAMEBUFFER_OFFSET;
	if (y < 0)
		return;
	if (y >= CAT_LCD_FRAMEBUFFER_H)
		return;
	
	const uint8_t* frame = sprite->frames[frame_idx];
	uint16_t run_idx = 0;
	uint16_t px_idx = y*CAT_LCD_FRAMEBUFFER_W+x;
	uint8_t dx = 0;
	uint8_t dy = 0;

	while(dy < CAT_TILE_SIZE)
	{
		uint8_t token = frame[run_idx++];
		uint8_t c_idx = token == 0xff ? frame[run_idx++] : token;
		uint16_t c = CAT_ADAPT_EMBEDDED_COLOUR(sprite->colour_table[c_idx]);
		uint8_t run_length = token == 0xff ? frame[run_idx++] : 1;
		uint8_t run_remainder = run_length;

		while(run_remainder > 0)
		{
			framebuffer[px_idx] = c;
			px_idx += 1;

			dx += 1;
			if (dx >= CAT_TILE_SIZE)
			{
				dx = 0;
				dy += 1;
				px_idx += CAT_LCD_FRAMEBUFFER_W-CAT_TILE_SIZE;
			}
			run_remainder -= 1;
		}
	}
}

void CAT_draw_tile_alpha(const CAT_sprite* sprite, int frame_idx, int x, int y)
{
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();
	
	y -= CAT_LCD_FRAMEBUFFER_OFFSET;
	if (y < 0)
		return;
	if (y >= CAT_LCD_FRAMEBUFFER_H)
		return;
	
	const uint8_t* frame = sprite->frames[frame_idx];
	uint16_t run_idx = 0;
	uint16_t px_idx = y*CAT_LCD_FRAMEBUFFER_W+x;
	uint8_t dx = 0;
	uint8_t dy = 0;
	uint8_t pitch = CAT_LCD_FRAMEBUFFER_W-CAT_TILE_SIZE;

	while(dy < CAT_TILE_SIZE)
	{
		uint8_t token = frame[run_idx++];
		uint8_t c_idx = token == 0xff ? frame[run_idx++] : token;
		uint16_t c = sprite->colour_table[c_idx];
		uint8_t run_length = token == 0xff ? frame[run_idx++] : 1;
		uint8_t run_remainder = run_length;

		if(c == CAT_TRANSPARENT)
		{
			uint16_t run = dx + run_length;
			uint8_t run_rows = run / CAT_TILE_SIZE;
			uint8_t run_off = run % CAT_TILE_SIZE;
			dy += run_rows;
			dx = run_off;
			px_idx += (run_rows * pitch) + run_length;
			continue;
		}
		c = CAT_ADAPT_EMBEDDED_COLOUR(c);

		while(run_remainder > 0)
		{
			framebuffer[px_idx] = c;
			px_idx += 1;

			dx += 1;
			if (dx >= CAT_TILE_SIZE)
			{
				dx = 0;
				dy += 1;
				px_idx += CAT_LCD_FRAMEBUFFER_W-CAT_TILE_SIZE;
			}
			run_remainder -= 1;
		}
	}
}