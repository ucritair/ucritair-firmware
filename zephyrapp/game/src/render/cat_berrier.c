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

	if (yi > LCD_FRAMEBUFFER_H || yf < 0)
		return;

	if (yf >= LCD_FRAMEBUFFER_H)
		yf = LCD_FRAMEBUFFER_H-1;

	if (yi < 0)
		yi = 0;

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			int idx = y * LCD_FRAMEBUFFER_W + x;
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
	uint16_t* end = framebuffer + LCD_FRAMEBUFFER_W * LCD_FRAMEBUFFER_H;
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

	if (yi > LCD_FRAMEBUFFER_H || yf < 0)
		return;

	if (yf > LCD_FRAMEBUFFER_H)
		yf = LCD_FRAMEBUFFER_H;

	if (yi < 0)
		yi = 0;

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			int idx = y * LCD_FRAMEBUFFER_W + x;
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
#ifdef CAT_EMBEDDED
			int xf = x - FRAMEBUFFER_ROW_OFFSET;
			if(y >= 0 && y < LCD_FRAMEBUFFER_W && xf >= 0 && xf < LCD_FRAMEBUFFER_H)
				framebuffer[xf * LCD_FRAMEBUFFER_W + y] = c;
#else
			if(y >= 0 && y < LCD_FRAMEBUFFER_W && x >= 0 && x < LCD_FRAMEBUFFER_H)
				framebuffer[x * LCD_FRAMEBUFFER_W + y] = c;
#endif

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
#ifdef CAT_EMBEDDED
			int yf = y - FRAMEBUFFER_ROW_OFFSET;
			if(x >= 0 && x < LCD_FRAMEBUFFER_W && yf >= 0 && yf < LCD_FRAMEBUFFER_H)
				framebuffer[yf * LCD_FRAMEBUFFER_W + x] = c;
#else
			if(x >= 0 && x < LCD_FRAMEBUFFER_W && y >= 0 && y < LCD_FRAMEBUFFER_H)
				framebuffer[y * LCD_FRAMEBUFFER_W + x] = c;
#endif

			err += d_err;
			if(err > dx)
			{
				y += y_step;
				err -= dx*2;
			}
		}
	}
}

#ifdef CAT_EMBEDDED
#define IF_IF_EMBEDDED(x) if (x)
#else
#define IF_IF_EMBEDDED(x)
#endif

void CAT_fillberry(int xi, int yi, int w, int h, uint16_t c)
{
	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	yi -= FRAMEBUFFER_ROW_OFFSET;
	int yf = yi + h;

	if (yi < 0) yi = 0;
	if (yf >= LCD_FRAMEBUFFER_H) yf = LCD_FRAMEBUFFER_H;

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xi+w; x++)
		{
			framebuffer[y * LCD_FRAMEBUFFER_W + x] = c;
		}
	}
}

void CAT_strokeberry(int xi, int yi, int w, int h, uint16_t c)
{
	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	yi -= FRAMEBUFFER_ROW_OFFSET;

	IF_IF_EMBEDDED(yi >= 0 && yi < LCD_FRAMEBUFFER_H)
	{
		for(int x = xi; x < xi + w; x++)
		{
			framebuffer[yi * LCD_FRAMEBUFFER_W + x] = c;
		}
	}

	int yf = yi + h;
	
	IF_IF_EMBEDDED((yf >= 0 && yf < LCD_FRAMEBUFFER_H))
	{
		for(int x = xi; x < xi + w; x++)
		{
			framebuffer[(yi+h-1) * LCD_FRAMEBUFFER_W + x] = c;
		}
	}

	for(int y = yi; y < yf; y++)
	{
		IF_IF_EMBEDDED(y >= 0 && y < LCD_FRAMEBUFFER_H)
		{
			framebuffer[y * LCD_FRAMEBUFFER_W + xi] = c;
			framebuffer[y * LCD_FRAMEBUFFER_W + (xi+w-1)] = c;
		}
	}
}

void CAT_rowberry(int x, int y, int w, uint16_t c)
{
	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	y -= FRAMEBUFFER_ROW_OFFSET;

	IF_IF_EMBEDDED(y >= 0 && y < LCD_FRAMEBUFFER_H)
	{
		memset(&framebuffer[y * LCD_FRAMEBUFFER_W + x], c, sizeof(uint16_t) * w);
	}
}