#include "cat_gizmos.h"

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