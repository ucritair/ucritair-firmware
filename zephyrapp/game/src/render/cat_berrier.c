#include "cat_render.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
// THE BERRIER

// CATs when they eat a
void CAT_greenberry(int xi, int w, int yi, int h, float t)
{
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	yi -= FRAMEBUFFER_ROW_OFFSET;
	int xf = xi + w * t;
	int yf = yi + h;

	if (yi > CAT_LCD_FRAMEBUFFER_H || yf < 0)
		return;

	if (yf >= CAT_LCD_FRAMEBUFFER_H)
		yf = CAT_LCD_FRAMEBUFFER_H-1;

	if (yi < 0)
		yi = 0;

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			int idx = y * CAT_LCD_FRAMEBUFFER_W + x;
			// r4 r3 r2 r1 r0 g5 g4 g3     g2 g1 g0 b4 b3 b2 b1 b0
			// g2 g1 g0 b4 b3 b2 b1 b0     r4 r3 r2 r1 r0 g5 g4 g3
			uint16_t px = framebuffer[idx];
			px |= 0b011;
			px &= (0b00010000<<8) | 0b10001111;
			framebuffer[idx] = ADAPT_EMBEDDED_COLOUR(px);
		}
	}
}
// Okay, it's more of an orangeberry. [Goldberry?](https://tolkiengateway.net/wiki/Goldberry)

void CAT_frameberry(uint16_t c)
{
	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	uint16_t* px = framebuffer;
	uint16_t* end = framebuffer + CAT_LCD_FRAMEBUFFER_W * CAT_LCD_FRAMEBUFFER_H;
	while(px != end)
	{
		*(px++) = c;
	}
}


// Cats would probably never eat this one
void CAT_greyberry(int xi, int w, int yi, int h)
{
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	yi -= FRAMEBUFFER_ROW_OFFSET;
	int xf = xi + w;
	int yf = yi + h;

	if (yi > CAT_LCD_FRAMEBUFFER_H || yf < 0)
		return;

	if (yf > CAT_LCD_FRAMEBUFFER_H)
		yf = CAT_LCD_FRAMEBUFFER_H;

	if (yi < 0)
		yi = 0;

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			int idx = y * CAT_LCD_FRAMEBUFFER_W + x;
			// r4 r3 r2 r1 r0 g5 g4 g3     g2 g1 g0 b4 b3 b2 b1 b0
			// g2 g1 g0 b4 b3 b2 b1 b0     r4 r3 r2 r1 r0 g5 g4 g3
			uint16_t px = framebuffer[idx];
			px &= (0b00010000<<8) | 0b10000100;
			framebuffer[idx] = ADAPT_EMBEDDED_COLOUR(px);
		}
	}
}

// implementation based on Dmitri Sokolov's
void CAT_lineberry(int xi, int yi, int xf, int yf, uint16_t c)
{
	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	// if the line is steep, transpose its start and end points
	bool steep = abs(yf-yi) > abs(xf-xi);
	if(steep)
	{
		int temp = xi;
		xi = yi;
		yi = temp;

		temp = xf;
		xf = yf;
		yf = temp;
	}

	// if the line heads left, swap its start and end points
	bool leftward = xi > xf;
	if(leftward)
	{
		int temp = xi;
		xi = xf;
		xf = temp;

		temp = yi;
		yi = yf;
		yf = temp;
	}
	
	int dx = xf - xi;
	int dy = yf - yi;

	// account for line heading up or down
	int y_step = (yf > yi) ? 1 : -1;
	int y = yi;
	
	// approximate d_err as abs(dy) / (dx ~= 0.5)
	int d_err = abs(dy) * 2;
	int err = 0;

	// if line is steep, we swap x,y in the draw call to undo our earlier transposition
	// we employ a branch between two for loops to avoid branching within one loop
	if(steep)
	{
		for(int x = xi; x < xf; x++)
		{
			int xw = x - FRAMEBUFFER_ROW_OFFSET;
			if(y >= 0 && y < CAT_LCD_FRAMEBUFFER_W && xw >= 0 && xw < CAT_LCD_FRAMEBUFFER_H)
				framebuffer[xw * CAT_LCD_FRAMEBUFFER_W + y] = c;

			err += d_err;
			if(err > dx)
			{
				y += y_step;
				err -= 2*dx;
			}
		}
	}
	else
	{
		for(int x = xi; x < xf; x++)
		{
			int yw = y - FRAMEBUFFER_ROW_OFFSET;
			if(x >= 0 && x < CAT_LCD_FRAMEBUFFER_W && yw >= 0 && yw < CAT_LCD_FRAMEBUFFER_H)
				framebuffer[yw * CAT_LCD_FRAMEBUFFER_W + x] = c;

			err += d_err;
			if(err > dx)
			{
				y += y_step;
				err -= dx*2;
			}
		}
	}
}

void CAT_fillberry(int xi, int yi, int w, int h, uint16_t c)
{
	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	int xf = clamp(xi + w, 0, CAT_LCD_FRAMEBUFFER_W);
	xi = clamp(xi, 0, CAT_LCD_FRAMEBUFFER_W);
	yi -= FRAMEBUFFER_ROW_OFFSET;
	int yf = clamp(yi + h, 0, CAT_LCD_FRAMEBUFFER_H);
	yi = clamp(yi, 0, CAT_LCD_FRAMEBUFFER_H);

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			framebuffer[y * CAT_LCD_FRAMEBUFFER_W + x] = c;
		}
	}
}

void CAT_strokeberry(int xi, int yi, int w, int h, uint16_t c)
{
	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	int xf = xi + w;
	yi -= FRAMEBUFFER_ROW_OFFSET;
	int yf = yi + h;
	
	if(yi >= 0 && yi < CAT_LCD_FRAMEBUFFER_H)
	{
		for(int x = xi; x < xf; x++)
		{
			if(x >= 0 && x < CAT_LCD_FRAMEBUFFER_W)
				framebuffer[yi * CAT_LCD_FRAMEBUFFER_W + x] = c;	
		}
	}
	if(yf > 0 && yf <= CAT_LCD_FRAMEBUFFER_H)
	{
		for(int x = xi; x < xf; x++)
		{
			if(x >= 0 && x < CAT_LCD_FRAMEBUFFER_W)
				framebuffer[(yf-1) * CAT_LCD_FRAMEBUFFER_W + x] = c;	
		}
	}

	if(xi >= 0 && xi < CAT_LCD_FRAMEBUFFER_W)
	{
		for(int y = yi; y < yf; y++)
		{
			if(y >= 0 && y < CAT_LCD_FRAMEBUFFER_H)
				framebuffer[y * CAT_LCD_FRAMEBUFFER_W + xi] = c;
		}
	}
	if(xf > 0 && xf <= CAT_LCD_FRAMEBUFFER_W)
	{
		for(int y = yi; y < yf; y++)
		{
			if(y >= 0 && y < CAT_LCD_FRAMEBUFFER_H)
				framebuffer[y * CAT_LCD_FRAMEBUFFER_W + (xf-1)] = c;
		}
	}
}

void CAT_rowberry(int x, int y, int w, uint16_t c)
{
	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	int xf = clamp(x+w, 0, CAT_LCD_FRAMEBUFFER_W);
	y -= FRAMEBUFFER_ROW_OFFSET;

	if(y >= 0 && y < CAT_LCD_FRAMEBUFFER_H)
	{
		for(int xw = x; xw < xf; xw++)
		{
			framebuffer[y * CAT_LCD_FRAMEBUFFER_W + xw] = c;
		}
	}
}

void CAT_pixberry(int x, int y, uint16_t c)
{
	if(x < 0 || x >= CAT_LCD_FRAMEBUFFER_W)
		return;
	y -= FRAMEBUFFER_ROW_OFFSET;
	if(y < 0 || y >= CAT_LCD_FRAMEBUFFER_H)
		return;

	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();
	framebuffer[y * CAT_LCD_FRAMEBUFFER_W + x] = c;
}

// Alright, this one is pushing it
void CAT_gizberry(int x, int y, CAT_sprite* stencil, uint16_t c, int flags)
{
	if(stencil == NULL)
	{
		CAT_printf("[ERROR] CAT_gizberry: null stencil\n");
		return;
	}

	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	int w = stencil->width;
	int h = stencil->height;
	if ((flags & CAT_DRAW_FLAG_CENTER_X) > 0)
		x -= w / 2;
	if ((flags & CAT_DRAW_FLAG_CENTER_Y) > 0)
		y -= h / 2;
	else if ((flags & CAT_DRAW_FLAG_BOTTOM) > 0)
		y -= h;
	int y_i = y;

	y -= FRAMEBUFFER_ROW_OFFSET;
	int y_f = y + h;
	if (y >= CAT_LCD_FRAMEBUFFER_H)
		return;
	if (y_f < 0)
		return;
	if (y_f > CAT_LCD_FRAMEBUFFER_H)
		y_f = CAT_LCD_FRAMEBUFFER_H;

	const uint8_t* frame = stencil->frames[0];
	int run_idx = 0;
	int dx = 0;
	bool valid_draw_region = true;

	while(valid_draw_region)
	{
		uint8_t token = frame[run_idx++];
		uint16_t colour_idx = token == 0xff ? frame[run_idx++] : token;
		uint16_t colour_565 = stencil->colour_table[colour_idx];
		if(colour_565 != 0xdead)
			colour_565 = ADAPT_EMBEDDED_COLOUR(colour_565);
		uint8_t run_length = token == 0xff ? frame[run_idx++] : 1;
		uint8_t run_remainder = run_length;
		
		while(run_remainder > 0)
		{
			int x_w =
			((draw_flags & CAT_DRAW_FLAG_REFLECT_X) > 0) ?
			x + w - dx : x + dx;

			if
			(
				colour_565 != 0xdead &&
				y >= 0 && x_w >= 0 && x_w < CAT_LCD_FRAMEBUFFER_W
			)
			{
				int px_idx = y * CAT_LCD_FRAMEBUFFER_W + x_w;
				framebuffer[px_idx] = c;
			}
			
			dx += 1;
			if (dx >= w)
			{
				dx = 0;
				y += 1;
				if(y >= y_f)
				{
					valid_draw_region = false;
					break;
				}
			}
			run_remainder -= 1;
		}
	}
}