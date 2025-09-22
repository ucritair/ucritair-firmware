#include "cat_gizmos.h"

#include "cat_curves.h"
#include "sprite_assets.h"

void CAT_draw_regular_polygon(int n, int x, int y, int r, float t, uint16_t c)
{
	t *= 2 * M_PI;
	float dt = 2 * M_PI / (float) n;
	int x0, y0, x1, y1;
	x0 = r*cosf(t);
	y0 = r*sinf(t);
	t += dt;
	for(int i = 1; i <= n; i++)
	{
		x1 = r*cosf(t);
		y1 = r*sinf(t);
		CAT_lineberry(x+x0, y+y0, x+x1, y+y1, c);
		x0 = x1;
		y0 = y1;
		t += dt;
	}
}

void CAT_draw_arrow(int x, int y, int w, int h, CAT_orientation d, uint16_t c)
{	
	int ox = x;
	int oy = y;
	int dx = w/2;
	int dy = h;

	switch (d)
	{
		case CAT_ORIENTATION_EAST:
			CAT_lineberry(ox, oy-dx, ox+dy, oy, c);
			CAT_lineberry(ox, oy+dx, ox+dy, oy, c);
		break;
		case CAT_ORIENTATION_NORTH:
			CAT_lineberry(ox-dx, oy, ox, oy-dy, c);
			CAT_lineberry(ox+dx, oy, ox, oy-dy, c);
		break;
		case CAT_ORIENTATION_WEST:
			CAT_lineberry(ox, oy-dx, ox-dy, oy, c);
			CAT_lineberry(ox, oy+dx, ox-dy, oy, c);
		break;
		case CAT_ORIENTATION_SOUTH:
			CAT_lineberry(ox-dx, oy, ox, oy+dy, c);
			CAT_lineberry(ox+dx, oy, ox, oy+dy, c);
		break;	
	}
}

void CAT_draw_cross(int x, int y, int w, int h, uint16_t c)
{
	int lx = x-w/2;
	int rx = x+w/2;
	int ty = y-h/2;
	int by = y+h/2;
	CAT_lineberry(lx, y, rx, y, c);
	CAT_lineberry(x, ty, x, by, c);
}

void CAT_draw_arrows(int x, int y, int size, int dist, uint16_t c)
{
	dist /= 2;
	int anchor_l = x - dist - size;
	int anchor_r = x + dist + size;
	CAT_draw_arrow(anchor_l, y, size*2, size, CAT_ORIENTATION_WEST, c);
	CAT_draw_arrow(anchor_r, y, size*2, size, CAT_ORIENTATION_EAST, c);
}

void CAT_draw_cross_box(int x0, int y0, int x1, int y1, uint16_t c)
{
	CAT_draw_cross(x0, y0, 4, 4, c);
	CAT_draw_cross(x1, y0, 4, 4, c);
	CAT_draw_cross(x1, y1, 4, 4, c);
	CAT_draw_cross(x0, y1, 4, 4, c);
}

void draw_corner(int x, int y, int xr, int yr, uint16_t c)
{
	CAT_lineberry(x, y, x+xr, y, c);
	CAT_lineberry(x, y, x, y+yr, c);
}

void CAT_draw_corner_box(int x0, int y0, int x1, int y1, uint16_t c)
{
	draw_corner(x0, y0, 4, 4, c);
	draw_corner(x1, y0, -4, 4, c);
	draw_corner(x1, y1, -4, -4, c);
	draw_corner(x0, y1, 4, -4, c);
}

void draw_corner_box(int x0, int y0, int x1, int y1, int r, uint16_t c)
{
	draw_corner(x0, y0, r, r, c);
	draw_corner(x1, y0, -r, r, c);
	draw_corner(x1, y1, -r, -r, c);
	draw_corner(x0, y1, r, -r, c);
}

void CAT_draw_progress_bar(int x, int y, int w, int h, uint16_t co, uint16_t ci, float t)
{
	CAT_strokeberry(x-w/2, y-h/2, w, h, co);
	if(t > 0)
	{
		t = CAT_ease_inout_sine(t);
		CAT_fillberry(x-w/2+2, y-h/2+2, (w-4)*t, h-4, ci);
	}
}

void CAT_draw_hexagon(int x, int y, int r, uint16_t c, float p)
{
	float dt = 2 * M_PI / 6.0f;
	float t = p;

	int x0, y0, x1, y1;
	x0 = r*cosf(t);
	y0 = r*sinf(t);
	t += dt;
	for(int i = 1; i <= 6; i++)
	{
		x1 = r*cosf(t);
		y1 = r*sinf(t);
		CAT_lineberry(x+x0, y+y0, x+x1, y+y1, c);
		x0 = x1;
		y0 = y1;
		t += dt;
	}
}

void CAT_draw_dot_grid(int x, int y, int w, int h, int s, uint16_t c)
{
	int xi = x+s;
	int yi = y+s;
	int xf = x+w*s;
	int yf = y+h*s;
	for(int yw = yi; yw < yf; yw += s)
	{
		for(int xw = xi; xw < xf; xw += s)
		{
			CAT_draw_cross(xw, yw, 1, 1, c);
		}
	}
}

#define PAGE_MARKER_SIZE 16
#define PAGE_MARKER_PADDING 2

void CAT_draw_page_markers(int y, int pages, int page, uint16_t c)
{
	int x0 = (CAT_LCD_SCREEN_W - ((PAGE_MARKER_SIZE + PAGE_MARKER_PADDING) * pages)) / 2;
	for(int i = 0; i < pages; i++)
	{
		int x = x0 + i * (PAGE_MARKER_SIZE + PAGE_MARKER_PADDING);
		CAT_set_sprite_colour(c);
		CAT_draw_sprite(&ui_radio_button_diamond_sprite, page == i, x, y);
	}
}

void CAT_draw_page_alert(int y, int pages, int page, uint16_t c)
{
	int x0 = (CAT_LCD_SCREEN_W - ((PAGE_MARKER_SIZE + PAGE_MARKER_PADDING) * pages)) / 2;
	int x = x0 + page * PAGE_MARKER_SIZE + PAGE_MARKER_PADDING;
	CAT_set_sprite_colour(c);
	CAT_draw_sprite(&ui_radio_button_diamond_sprite, 0, x, y);
	if(CAT_pulse(0.25f))
	{
		CAT_set_sprite_colour(c);
		CAT_draw_sprite(&ui_radio_button_diamond_sprite, 1, x, y);
		CAT_draw_regular_polygon(4, x+8, y+8, 16, 0.25f, CAT_RED);
	}
}

#define SUBPAGE_MARKER_R 2
#define SUBPAGE_MARKER_PADDING 8

void CAT_draw_subpage_markers(int y, int pages, int page, uint16_t c)
{
	int start_x = CAT_LCD_SCREEN_W/2 - (((2*SUBPAGE_MARKER_R) * pages) + (SUBPAGE_MARKER_PADDING * (pages-1))) / 2;
	int x = start_x;

	for(int i = 0; i < pages; i++)
	{
		if(i == page)
			CAT_discberry(x, y, SUBPAGE_MARKER_R, CAT_WHITE);
		else
			CAT_circberry(x, y, SUBPAGE_MARKER_R, CAT_WHITE);
		x += 2 * SUBPAGE_MARKER_R + SUBPAGE_MARKER_PADDING;
	}
}

void CAT_draw_arrow_slider(int x0, int y0, int x1, int y1, float t, uint16_t c)
{
	int r = 4;
	int d = r*2;

	CAT_circberry(x0, y0, r, c);
	CAT_circberry(x1, y1, r, c);

	float dx = x1-x0;
	float dy = y1-y0;
	float l = sqrt(dx*dx + dy*dy);
	dx /= l;
	dy /= l;
	
	CAT_lineberry
	(
		x0+dx*r, y0+dy*r,
		x1-dx*r, y1-dy*r,
		c
	);

	float dxn = -dy;
	float dyn = dx;
	
	int cx = x0 + dx*r + dx*(l-d)*t;
	int cy = y0 + dy*r + dy*(l-d)*t;

	int ax = cx;
	int ay = cy;
	ax -= dx * r;
	ay -= dy * r;
	ax += dxn * r;
	ay += dyn * r;
	int bx = ax - (dxn*d);
	int by = ay - (dyn*d);

	CAT_lineberry(ax, ay, cx, cy, c);
	CAT_lineberry(bx, by, cx, cy, c);
}

void CAT_draw_empty_sfx(int x0, int y0, int a, int b, uint16_t c)
{
	int parts = 32;
	float arc = 2 * M_PI / parts;

	for(int i = 0; i < parts; i++)
	{
		float t = arc * i;
		int x = x0 + a * CAT_cos(t);
		int y = y0 + b * CAT_sin(t);
		CAT_vec2 N = CAT_vec2_rotate((CAT_vec2){1, 0}, t);
		CAT_lineberry(x, y, x + N.x * 4, y + N.y * 4, c);
	}
}
