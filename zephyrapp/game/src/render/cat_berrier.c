#include "cat_render.h"

#include <string.h>
#include "sprite_assets.h"
#include <math.h>

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
	x = clamp(x, 0, CAT_LCD_FRAMEBUFFER_W);
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

void CAT_circberry(int x, int y, int r, uint16_t c)
{
	//uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	int f = 1 - r;
	int ddfx = 0;
	int ddfy = -2 * r;
	int dx = 0;
	int dy = r;

	CAT_pixberry(x, y + r, c);
	CAT_pixberry(x, y - r, c);
	CAT_pixberry(x + r, y, c);
	CAT_pixberry(x - r, y, c);

	while(dx < dy)
	{
		if(f >= 0)
		{
			dy--;
			ddfy += 2;
			f += ddfy;
		}

		dx++;
		ddfx += 2;
		f += ddfx + 1;

		CAT_pixberry(x + dx, y + dy, c);
        CAT_pixberry(x - dx, y + dy, c);
        CAT_pixberry(x + dx, y - dy, c);
        CAT_pixberry(x - dx, y - dy, c);
        CAT_pixberry(x + dy, y + dx, c);
        CAT_pixberry(x - dy, y + dx, c);
        CAT_pixberry(x + dy, y - dx, c);
        CAT_pixberry(x - dy, y - dx, c);
	}
}

void CAT_discberry(int x, int y, int r, uint16_t c)
{
	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	y -= FRAMEBUFFER_ROW_OFFSET;
	int yi = y - r;
	int yf = y + r;
	if (yi > CAT_LCD_FRAMEBUFFER_H || yf < 0)
		return;

	for(int dy = -r; dy < r; dy++)
	{
		int y_w = y + dy;
		if(y_w < 0 || y_w >= CAT_LCD_FRAMEBUFFER_H)
			continue;

		for(int dx = -r; dx < r; dx++)
		{
			int x_w = x + dx;
			if(x_w < 0 || x_w >= CAT_LCD_FRAMEBUFFER_W)
				continue;

			int L = dx*dx+dy*dy;
			int R = r*r;
			if(L <= R)
				framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x_w] = c;
		}
	}
}

void CAT_ringberry(int x, int y, int R, int r, uint16_t c, float t)
{
	c = ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	y -= FRAMEBUFFER_ROW_OFFSET;
	int yi = y - r;
	int yf = y + r;
	if (yi > CAT_LCD_FRAMEBUFFER_H || yf < 0)
		return;

	for(int dy = -R; dy < R; dy++)
	{
		int y_w = y + dy;
		if(y_w < 0 || y_w >= CAT_LCD_FRAMEBUFFER_H)
			continue;

		for(int dx = -R; dx < R; dx++)
		{
			int x_w = x + dx;
			if(x_w < 0 || x_w >= CAT_LCD_FRAMEBUFFER_W)
				continue;

			if((dx*dx+dy*dy) <= R*R && (dx*dx+dy*dy) >= r*r)
			{
				float arc = atan2f((float) dy, (float) dx) + M_PI - (M_PI / 8);
				if(arc <= t * M_PI * 2)
				{
					framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x_w] = c;
				}
			}
		}
	}
}

void CAT_polyberry(int x, int y, int* poly, int count, uint16_t c, CAT_poly_mode mode)
{
	int i = 0;
	int stride = mode == CAT_POLY_MODE_LINES ? 2 : 1;
	while(i < count-1)
	{
		int x0 = poly[i*2+0]; int y0 = poly[i*2+1];
		int x1 = poly[(i+1)*2+0]; int y1 = poly[(i+1)*2+1];
		CAT_lineberry(x+x0, y+y0, x+x1, y+y1, c);
		i += stride;
	}
	if(mode == CAT_POLY_MODE_LINE_LOOP)
	{
		int x0 = poly[i*2+0]; int y0 = poly[i*2+1];
		int x1 = poly[0]; int y1 = poly[1];
		CAT_lineberry(x+x0, y+y0, x+x1, y+y1, c);
	}
}