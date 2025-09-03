#include "cat_gizmos.h"

#include "cat_curves.h"
#include "sprite_assets.h"

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
	if(t < 0)
		return;
	float r = lerp(r0, r1, t/T);
	float theta = p + w * t;
	CAT_draw_gizmo_primitive(primitive, x, y, r, theta, c);
}

#define PAGE_MARKER_SIZE 16
#define PAGE_MARKER_PADDING 2

void CAT_draw_page_markers(int y, int pages, int page, uint16_t c)
{
	int x0 = (CAT_LCD_SCREEN_W - ((PAGE_MARKER_SIZE + PAGE_MARKER_PADDING) * pages)) / 2;
	for(int i = 0; i < pages; i++)
	{
		int x = x0 + i * PAGE_MARKER_SIZE + PAGE_MARKER_PADDING;
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
		CAT_draw_gizmo_primitive(CAT_GIZMO_PRIMITIVE_BOX, x+8, y+8, 16, 0.25f, CAT_RED);
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

void CAT_draw_delta_arrow(int x, int y, bool up, uint16_t c)
{
	int y0 = up ? y-12 : y;
	int y1 = up ? y : y-12;
	CAT_lineberry(x, y0, x+8, y1, c);
	CAT_lineberry(x+8, y1, x+16, y0, c);
}
