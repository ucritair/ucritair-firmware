#include "cat_render.h"

#include <string.h>
#include "sprite_assets.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////
// THE BERRIER

void CAT_rowberry(int y0, int y1, uint16_t c)
{
	y0 -= CAT_LCD_FRAMEBUFFER_OFFSET;
	if(y0 >= CAT_LCD_FRAMEBUFFER_H)
		return;
	y0 = CAT_max(y0, 0);

	y1 -= CAT_LCD_FRAMEBUFFER_OFFSET;
	if(y1 <= 0)
		return;
	y1 = CAT_min(y1, CAT_LCD_FRAMEBUFFER_H);

	c = CAT_ADAPT_DESKTOP_COLOUR(c);
	int i0 = y0 * 120;
	int i1 = y1 * 120;
	
	uint32_t c32 = (c << 16) | c;
	uint32_t* px = &((uint32_t*) CAT_LCD_get_framebuffer())[i0];
	for(int i = i0; i < i1 && i < 19200; i++)
		*(px++) = c32;
}

// CATs when they eat a frameberry
void CAT_frameberry(uint16_t c)
{
	c = CAT_ADAPT_DESKTOP_COLOUR(c);
	uint32_t c32 = (c << 16) | c;
	uint32_t* px = CAT_LCD_get_framebuffer();
	for(int i = 0; i < 19200; i++)
		*(px++) = c32;
}

// CATs when they eat a lineberry
void CAT_lineberry(int xi, int yi, int xf, int yf, uint16_t c)
{
	c = CAT_ADAPT_DESKTOP_COLOUR(c);
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
		for(int x = xi; x <= xf; x++)
		{
			int xw = x - CAT_LCD_FRAMEBUFFER_OFFSET;
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
		for(int x = xi; x <= xf; x++)
		{
			int yw = y - CAT_LCD_FRAMEBUFFER_OFFSET;
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

// CATs when they eat a fillberry
void CAT_fillberry(int xi, int yi, int w, int h, uint16_t c)
{
	c = CAT_ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	int xf = CAT_clamp(xi + w, 0, CAT_LCD_FRAMEBUFFER_W);
	xi = CAT_clamp(xi, 0, CAT_LCD_FRAMEBUFFER_W);
	yi -= CAT_LCD_FRAMEBUFFER_OFFSET;
	int yf = CAT_clamp(yi + h, 0, CAT_LCD_FRAMEBUFFER_H);
	yi = CAT_clamp(yi, 0, CAT_LCD_FRAMEBUFFER_H);

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			framebuffer[y * CAT_LCD_FRAMEBUFFER_W + x] = c;
		}
	}
}

// CATs when they eat a strokeberry
void CAT_strokeberry(int xi, int yi, int w, int h, uint16_t c)
{
	c = CAT_ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	int xf = xi + w;
	yi -= CAT_LCD_FRAMEBUFFER_OFFSET;
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

// CATs when they eat a pixberry
void CAT_pixberry(int x, int y, uint16_t c)
{
	if(x < 0 || x >= CAT_LCD_FRAMEBUFFER_W)
		return;
	y -= CAT_LCD_FRAMEBUFFER_OFFSET;
	if(y < 0 || y >= CAT_LCD_FRAMEBUFFER_H)
		return;

	c = CAT_ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();
	framebuffer[y * CAT_LCD_FRAMEBUFFER_W + x] = c;
}

// CATs when they eat a circberry
void CAT_circberry(int x, int y, int r, uint16_t c)
{
	//uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	int min_x = x-r;
	int max_x = x+r;
	int min_y = y-r - CAT_LCD_FRAMEBUFFER_OFFSET;
	int max_y = y+r - CAT_LCD_FRAMEBUFFER_OFFSET;
	if(min_x >= CAT_LCD_FRAMEBUFFER_W)
		return;
	if(max_x < 0)
		return;
	if(min_y >= CAT_LCD_FRAMEBUFFER_H)
		return;
	if(max_y < 0)
		return;

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

// CATs when they eat a discberry
void CAT_discberry(int x, int y, int r, uint16_t c)
{
	int min_x = x-r;
	int max_x = x+r;
	int min_y = y-r - CAT_LCD_FRAMEBUFFER_OFFSET;
	int max_y = y+r - CAT_LCD_FRAMEBUFFER_OFFSET;
	if(min_x >= CAT_LCD_FRAMEBUFFER_W)
		return;
	if(max_x < 0)
		return;
	if(min_y >= CAT_LCD_FRAMEBUFFER_H)
		return;
	if(max_y < 0)
		return;

	int f = 1 - r;
	int ddfx = 0;
	int ddfy = -2 * r;
	int dx = 0;
	int dy = r;

	CAT_lineberry(x-r, y, x+r, y, c);

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

		CAT_lineberry(x - dx, y + dy, x + dx, y + dy, c);
        CAT_lineberry(x - dx, y - dy, x + dx, y - dy, c);
        CAT_lineberry(x - dy, y + dx, x + dy, y + dx, c);
        CAT_lineberry(x - dy, y - dx, x + dy, y - dx, c);
	}
}

// CATs when they eat an annulusberry
void CAT_annulusberry(int x, int y, int R, int r, uint16_t c, float t, float shift)
{
	c = CAT_ADAPT_DESKTOP_COLOUR(c);
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	y -= CAT_LCD_FRAMEBUFFER_OFFSET;
	int yi = y - r;
	int yf = y + r;
	if (yi > CAT_LCD_FRAMEBUFFER_H || yf < 0)
		return;

	t *= 2 * M_PI;
	shift *= 2 * M_PI;

	float cos_s = cosf(shift);
	float sin_s = sinf(shift);

	for(int dy = -R; dy <= R; dy++)
	{
		int y_w = y + dy;
		if(y_w < 0 || y_w >= CAT_LCD_FRAMEBUFFER_H)
			continue;

		for(int dx = -R; dx <= R; dx++)
		{
			int x_w = x + dx;
			if(x_w < 0 || x_w >= CAT_LCD_FRAMEBUFFER_W)
				continue;

			int length2 = dx*dx + dy*dy;
			if(length2 >= r*r && length2 <= R*R)
			{
				float dxr = shift == 0 ? dx : cos_s * dx - sin_s * dy;
				float dyr = shift == 0 ? dy : sin_s * dx + cos_s * dy;
				float arc = atan2f(dyr, -dxr) + M_PI;
				if(arc <= t)
					framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x_w] = c;				
			}
		}
	}
}
