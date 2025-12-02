#include "cat_gizmos.h"

#include "cat_curves.h"
#include "sprite_assets.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_spriter.h"

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
		CAT_fillberry(x-w/2+2, y-h/2+2, (w-4)*t, h-4, ci);
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

void CAT_draw_score_line(int x, int y, int w, float t, float dt, uint16_t c, uint16_t ct, uint16_t cd)
{
	int x1 = x + roundf(w * t);
	int x2 = x + roundf(w * (t+dt));
	int x3 = x + w;

	CAT_lineberry(x, y, x1, y, ct);
	if(x2 != x1)
		CAT_lineberry(x1, y, x2, y, cd);
	if(x3 != x1)
		CAT_lineberry(x2, y, x3, y, c);
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
	int x = x0 + page * (PAGE_MARKER_SIZE + PAGE_MARKER_PADDING);
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


void CAT_draw_lock(int x, int y, int r, float t, uint16_t c)
{
	int deltas[10] =
	{
		0, -1,
		1, 0,
		0, 1,
		-1, 0,
		0, -1
	};

	t = CAT_clamp(t, 0, 1);
	int idx = t*4;
	float frac = (t*4)-idx;

	CAT_set_text_colour(c);
	int glyph_scale = CAT_max(CAT_sqrt((r*r)/2)/CAT_GLYPH_HEIGHT, 1);
	CAT_set_text_scale(glyph_scale);
	CAT_draw_text(x-glyph_scale*CAT_GLYPH_WIDTH/2, y-glyph_scale*CAT_GLYPH_HEIGHT/2, "A");

	float scale = r * 0.75f;
	for(int i = 0; i < 4; i++)
	{
		int dx0 = deltas[i*2+0] * scale;
		int dy0 = deltas[i*2+1] * scale;
		int dx1 = deltas[i*2+2] * scale;
		int dy1 = deltas[i*2+3] * scale;
		CAT_lineberry(x+dx0, y+dy0, x+dx1, y+dy1, c);
	}

	if(t <= 0)
		return;

	scale = t >= 1 ? r * 1.25f : r;
	for(int i = 0; i < idx; i++)
	{
		int dx0 = deltas[i*2+0] * scale;
		int dy0 = deltas[i*2+1] * scale;
		int dx1 = deltas[i*2+2] * scale;
		int dy1 = deltas[i*2+3] * scale;
		CAT_lineberry(x+dx0, y+dy0, x+dx1, y+dy1, c);
	}

	int dx0 = deltas[idx*2+0] * scale;
	int dy0 = deltas[idx*2+1] * scale;
	int dx1 = deltas[idx*2+2] * scale;
	int dy1 = deltas[idx*2+3] * scale;
	int x0 = x+dx0;
	int y0 = y+dy0;
	int x1 = x+dx1;
	int y1 = y+dy1;
	float dx = x1-x0;
	float dy = y1-y0;
	x1 = x0 + dx * frac;
	y1 = y0 + dy * frac;
	CAT_lineberry(x0, y0, x1, y1, c);
}

void CAT_draw_dpad(int x, int y, int R, int mask, uint16_t cf, uint16_t cb)
{
	static int bits[] =
	{
		CAT_BUTTON_BIT(CAT_BUTTON_RIGHT),
		CAT_BUTTON_BIT(CAT_BUTTON_UP),
		CAT_BUTTON_BIT(CAT_BUTTON_LEFT),
		CAT_BUTTON_BIT(CAT_BUTTON_DOWN),
	};

	int hypot = CAT_sqrt(2)*R;
	int max_r = hypot/2;
	int min_r = CAT_min(max_r, 8);
	int ideal_r = hypot/3;
	int r = CAT_clamp(ideal_r, min_r, max_r);

	CAT_circberry(x+R, y, r, cf);
	CAT_circberry(x, y-R, r, cf);
	CAT_circberry(x-R, y, r, cf);
	CAT_circberry(x, y+R, r, cf);

	int aw = CAT_max(r, 0);
	int ah = aw/2;
	int ar = ah/4;

	if(mask & CAT_BUTTON_BIT(CAT_BUTTON_RIGHT))
	{
		CAT_discberry(x+R, y, r, cf);
		CAT_draw_arrow(x+R-ar, y, aw, ah, CAT_ORIENTATION_EAST, cb);
	}
	if(mask & CAT_BUTTON_BIT(CAT_BUTTON_UP))
	{
		CAT_discberry(x, y-R, r, cf);
		CAT_draw_arrow(x, y-R+ar, aw, ah, CAT_ORIENTATION_NORTH, cb);
	}
	if(mask & CAT_BUTTON_BIT(CAT_BUTTON_LEFT))
	{
		CAT_discberry(x-R, y, r, cf);
		CAT_draw_arrow(x-R+ar, y, aw, ah, CAT_ORIENTATION_WEST, cb);
	}
	if(mask & CAT_BUTTON_BIT(CAT_BUTTON_DOWN))
	{
		CAT_discberry(x, y+R, r, cf);
		CAT_draw_arrow(x, y+R-ar, aw, ah, CAT_ORIENTATION_SOUTH, cb);
	}
}

void CAT_draw_star(int x, int y, int r, uint16_t c)
{
	int r0 = r/2;
	int r1 = r;

	float dt = 2 * M_PI / (float) 10;
	float t = M_PI/2 - dt;
	int x0, y0, x1, y1;

	x0 = r*CAT_cos(t);
	y0 = r*CAT_sin(t);
	t += dt;

	for(int i = 1; i <= 10; i++)
	{
		float rp = !(i & 1) ? r1 : r0;
			
		x1 = rp*CAT_cos(t);
		y1 = rp*CAT_sin(t);
		CAT_lineberry(x+x0, y+y0, x+x1, y+y1, c);

		x0 = x1;
		y0 = y1;
		t += dt;
	}
}

int CAT_draw_button(int x, int y, int button, uint16_t c)
{
	static uint8_t pad = 2;

	switch (button)
	{
		case CAT_BUTTON_A:
			CAT_draw_tinyglyph(x+pad, y+pad, 'A', c);
			CAT_strokeberry(x, y, 8+pad*2, 8+pad*2, c);
			return x+8+pad*2;
		case CAT_BUTTON_B:
			CAT_draw_tinyglyph(x+pad, y+pad, 'B', c);
			CAT_strokeberry(x, y, 8+pad*2, 8+pad*2, c);
			return x+8+pad*2;

		case CAT_BUTTON_RIGHT:
			CAT_draw_tinyglyph(x+pad, y+pad, '\6', c);
			CAT_strokeberry(x, y, 8+pad*2, 8+pad*2, c);
			return x+8+pad*2;
		case CAT_BUTTON_UP:
			CAT_draw_tinyglyph(x+pad, y+pad, '\7', c);
			CAT_strokeberry(x, y, 8+pad*2, 8+pad*2, c);
			return x+8+pad*2;
		case CAT_BUTTON_LEFT:
			CAT_draw_tinyglyph(x+pad, y+pad, '\10', c);
			CAT_strokeberry(x, y, 8+pad*2, 8+pad*2, c);
			return x+8+pad*2;
		case CAT_BUTTON_DOWN:
			CAT_draw_tinyglyph(x+pad, y+pad, '\11', c);
			CAT_strokeberry(x, y, 8+pad*2, 8+pad*2, c);
			return x+8+pad*2;

		case CAT_BUTTON_START:
			CAT_draw_tinystring(x+pad, y+pad, "STRT", c);
			CAT_strokeberry(x, y, 8*4+pad*2, 8+pad*2, c);
			return x+8*4+pad*2;
		case CAT_BUTTON_SELECT:
			CAT_draw_tinystring(x+pad, y+pad, "SLCT", c);
			CAT_strokeberry(x, y, 8*4+pad*2, 8+pad*2, c);
			return x+8*4+pad*2;

		default: return 0;
	}
}