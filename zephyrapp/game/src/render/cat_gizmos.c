#include "cat_gizmos.h"

#include "cat_curves.h"

void CAT_draw_arrows(int x, int y, int size, int dist, uint16_t c)
{
	dist /= 2;
	int anchor_l = x - dist - size;
	int anchor_r = x + dist + size;
	CAT_lineberry(anchor_l, y, anchor_l + size, y + size, c);
	CAT_lineberry(anchor_l, y, anchor_l + size, y - size, c);
	CAT_lineberry(anchor_r, y, anchor_r - size, y + size, c);
	CAT_lineberry(anchor_r, y, anchor_r - size, y - size, c);
}

void draw_cross(int x, int y, int r, uint16_t c)
{
	CAT_lineberry(x-r, y, x+r, y, c);
	CAT_lineberry(x, y-r, x, y+r, c);
}

void CAT_draw_cross_box(int x0, int y0, int x1, int y1, uint16_t c)
{
	draw_cross(x0, y0, 4, c);
	draw_cross(x1, y0, 4, c);
	draw_cross(x1, y1, 4, c);
	draw_cross(x0, y1, 4, c);
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
	x0 = r*cos(t);
	y0 = r*sin(t);
	t += dt;
	for(int i = 1; i <= 6; i++)
	{
		x1 = r*cos(t);
		y1 = r*sin(t);
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
			draw_cross(xw, yw, 1, c);
		}
	}
}

void CAT_draw_corner_explosion(int x, int y, int s1, int s2, uint16_t c, float t)
{
	draw_corner_box(x-(s2-s1), y-(s2-s1), x+s2, y+s2, 16, c);
	for(int i = 0; i < 3; i++)
	{
		int s = lerp(s1, s2, i / 2.0f * t);
		int off = s-s1;
		int r = lerp(4, 16, i / 2.0f * t);
		draw_corner_box(x-off, y-off, x+s, y+s, r, c);
	}
}

void CAT_draw_regular_polygon(int n, int x, int y, int r, float t, uint16_t c)
{
	t *= 2 * M_PI;
	float dt = 2 * M_PI / (float) n;
	int x0, y0, x1, y1;
	x0 = r*cos(t);
	y0 = r*sin(t);
	t += dt;
	for(int i = 1; i <= n; i++)
	{
		x1 = r*cos(t);
		y1 = r*sin(t);
		CAT_lineberry(x+x0, y+y0, x+x1, y+y1, c);
		x0 = x1;
		y0 = y1;
		t += dt;
	}
}

void CAT_draw_gizmo_primitive(CAT_gizmo_primitive primitive, int x, int y, int r, float t, uint16_t c)
{
	switch (primitive)
	{
		case CAT_GIZMO_PRIMITIVE_RING:
			CAT_circberry(x, y, r, c);
		break;

		case CAT_GIZMO_PRIMITIVE_TRI:	
			CAT_draw_regular_polygon(3, x, y, r, t, c);
		break;

		case CAT_GIZMO_PRIMITIVE_BOX:
			CAT_draw_regular_polygon(4, x, y, r, t, c);
		break;

		case CAT_GIZMO_PRIMITIVE_HEX:
			CAT_draw_regular_polygon(6, x, y, r, t, c);
		break;
	}
}

void CAT_draw_ripple(CAT_gizmo_primitive primitive, int x, int y, float r0, float r1, float p, float w, float t, float T, uint16_t c)
{
	float r = lerp(r0, r1, t/T);
	float theta = p + w * t;
	CAT_draw_gizmo_primitive(primitive, x, y, r, theta, c);
}
