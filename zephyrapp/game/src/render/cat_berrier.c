#include "cat_render.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
// THE BERRIER

uint8_t luminance(uint16_t rgb)
{
	uint8_t r = ((rgb & 0b1111100000000000) >> 11);
	uint8_t g = (rgb & 0b0000011111100000) >> 5;
	uint8_t b = rgb & 0b0000000000011111;
	uint8_t l = ((r << 1) + r  + (g << 2) + b) >> 1;
	return l;
}

// CATs when they eat a
void CAT_greenberry(int xi, int w, int yi, int h, float t)
{
#ifdef CAT_EMBEDDED
	yi -= framebuffer_offset_h;
#endif

	int xf = xi + w * t;
	int yf = yi + h;

#ifdef CAT_EMBEDDED
	if (yi > LCD_FRAMEBUFFER_H || yf < 0)
		return;

	if (yf >= LCD_FRAMEBUFFER_H)
		yf = LCD_FRAMEBUFFER_H-1;

	if (yi < 0)
		yi = 0;
#endif

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			int idx = y * LCD_SCREEN_W + x;
#ifdef CAT_DESKTOP
			uint16_t c = FRAMEBUFFER[idx];
			uint8_t l = luminance(c);
			FRAMEBUFFER[idx] = RGB8882565(l >> 1, l, l >> 1);
#else
			// r4 r3 r2 r1 r0 g5 g4 g3     g2 g1 g0 b4 b3 b2 b1 b0
			// g2 g1 g0 b4 b3 b2 b1 b0     r4 r3 r2 r1 r0 g5 g4 g3
			uint16_t px = FRAMEBUFFER[idx];

			px |= 0b011;
			px &= (0b00010000<<8) | 0b10001111;

			FRAMEBUFFER[idx] = px;
#endif
		}
	}
}
// Okay, it's more of an orangeberry. [Goldberry?](https://tolkiengateway.net/wiki/Goldberry)

void CAT_frameberry(uint16_t c)
{
#ifdef CAT_EMBEDDED
	c = (c >> 8) | ((c & 0xff) << 8);
#endif

	uint16_t* px = FRAMEBUFFER;
	uint16_t* end = FRAMEBUFFER + LCD_SCREEN_W * LCD_FRAMEBUFFER_H;
	while(px != end)
	{
		*(px++) = c;
	}
}


// Cats would probably never eat this one
void CAT_greyberry(int xi, int w, int yi, int h)
{
#ifdef CAT_EMBEDDED
	yi -= framebuffer_offset_h;
#endif

	int xf = xi + w;
	int yf = yi + h;

#ifdef CAT_EMBEDDED
	if (yi > LCD_FRAMEBUFFER_H || yf < 0)
		return;

	if (yf > LCD_FRAMEBUFFER_H)
		yf = LCD_FRAMEBUFFER_H;

	if (yi < 0)
		yi = 0;
#endif

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			int idx = y * LCD_SCREEN_W + x;
#ifdef CAT_DESKTOP
			uint16_t c = FRAMEBUFFER[idx];
			uint8_t l = luminance(c);
			FRAMEBUFFER[idx] = RGB8882565(l, l, l);
#else
			// r4 r3 r2 r1 r0 g5 g4 g3     g2 g1 g0 b4 b3 b2 b1 b0
			// g2 g1 g0 b4 b3 b2 b1 b0     r4 r3 r2 r1 r0 g5 g4 g3
			uint16_t px = FRAMEBUFFER[idx];
			px &= (0b00010000<<8) | 0b10000100;
			FRAMEBUFFER[idx] = px;
#endif
		}
	}
}

// implementation based on Dmitri Sokolov's
void CAT_lineberry(int xi, int yi, int xf, int yf, uint16_t c)
{
#ifdef CAT_EMBEDDED
	c = (c >> 8) | ((c & 0xff) << 8);
#endif

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
			int xf = x - framebuffer_offset_h;
			if(y >= 0 && y < LCD_FRAMEBUFFER_W && xf >= 0 && xf < LCD_FRAMEBUFFER_H)
				FRAMEBUFFER[xf * LCD_FRAMEBUFFER_W + y] = c;
#else
			if(y >= 0 && y < LCD_SCREEN_W && x >= 0 && x < LCD_SCREEN_H)
				FRAMEBUFFER[x * LCD_SCREEN_W + y] = c;
#endif

			err += d_err;
			if(err > dx)
			{
				y += y_step;
				err -= dx*2;
			}
		}
	}
	else
	{
		for(int x = xi; x < xf; x++)
		{
#ifdef CAT_EMBEDDED
			int yf = y - framebuffer_offset_h;
			if(x >= 0 && x < LCD_FRAMEBUFFER_W && yf >= 0 && yf < LCD_FRAMEBUFFER_H)
				FRAMEBUFFER[yf * LCD_FRAMEBUFFER_W + x] = c;
#else
			if(x >= 0 && x < LCD_SCREEN_W && y >= 0 && y < LCD_SCREEN_H)
				FRAMEBUFFER[y * LCD_SCREEN_W + x] = c;
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
	int yend = yi + h;

#ifdef CAT_EMBEDDED
	c = (c >> 8) | ((c & 0xff) << 8);
	yi -= framebuffer_offset_h;
	yend -= framebuffer_offset_h;

	if (yi < 0) yi = 0;
	if (yend >= LCD_FRAMEBUFFER_H) yend = LCD_FRAMEBUFFER_H;
#endif

	for(int y = yi; y < yend; y++)
	{
		for(int x = xi; x < xi+w; x++)
		{
			FRAMEBUFFER[y * LCD_SCREEN_W + x] = c;
		}
	}
}

void CAT_strokeberry(int xi, int yi, int w, int h, uint16_t c)
{
#ifdef CAT_EMBEDDED
	c = (c >> 8) | ((c & 0xff) << 8);
	yi -= framebuffer_offset_h;
#endif

	IF_IF_EMBEDDED(yi >= 0 && yi < LCD_FRAMEBUFFER_H)
	{
		for(int x = xi; x < xi + w; x++)
		{
			FRAMEBUFFER[yi * LCD_SCREEN_W + x] = c;
		}
	}

	int y2 = yi+h-1;

	IF_IF_EMBEDDED((y2 >= 0 && y2 < LCD_FRAMEBUFFER_H))
	{
		for(int x = xi; x < xi + w; x++)
		{
			FRAMEBUFFER[(yi+h-1) * LCD_SCREEN_W + x] = c;
		}
	}

	for(int y = yi; y < yi + h; y++)
	{
		IF_IF_EMBEDDED(y >= 0 && y < LCD_FRAMEBUFFER_H)
		{
			FRAMEBUFFER[y * LCD_SCREEN_W + xi] = c;
			FRAMEBUFFER[y * LCD_SCREEN_W + (xi+w-1)] = c;
		}
	}
}

void CAT_rowberry(int x, int y, int w, uint16_t c)
{
#ifdef CAT_EMBEDDED
	y -= framebuffer_offset_h;
	c = (c >> 8) | ((c & 0xff) << 8);
#endif
	IF_IF_EMBEDDED(y >= 0 && y < LCD_FRAMEBUFFER_H)
	{
		memset(&FRAMEBUFFER[y * LCD_SCREEN_W + x], c, sizeof(uint16_t) * w);
	}
}