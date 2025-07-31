#include "cat_graph.h"

#include "cat_render.h"
#include <math.h>
#include "cat_math.h"

static int wdw_x0 = 0;
static int wdw_y0 = 0;
static int wdw_x1 = CAT_LCD_SCREEN_W-1;
static int wdw_y1 = CAT_LCD_SCREEN_H-1;

static float vp_x0 = 0;
static float vp_y0 = 0;
static float vp_x1 = 1;
static float vp_y1 = 1;

static bool auto_viewport = false;
static uint16_t bg_colour = CAT_TRANSPARENT;
static uint16_t fg_colour = CAT_WHITE;
static uint8_t pt_size = 0;
static bool pt_fill = false;

void CAT_graph_set_window(int x0, int y0, int x1, int y1)
{
	wdw_x0 = x0;
	wdw_y0 = y0;
	wdw_x1 = x1;
	wdw_y1 = y1;
}

void CAT_graph_set_viewport(float x0, float y0, float x1, float y1)
{
	vp_x0 = x0;
	vp_y0 = y0;
	vp_x1 = x1;
	vp_y1 = y1;
}

void CAT_graph_set_auto_viewport(bool toggle)
{
	auto_viewport = toggle;
}

void CAT_graph_set_background(uint16_t colour)
{
	bg_colour = colour;
}

void CAT_graph_set_foreground(uint16_t colour)
{
	fg_colour = colour;
}

void CAT_graph_set_point_size(uint8_t size)
{
	pt_size = size;
}

void CAT_graph_set_point_fill(bool toggle)
{
	pt_fill = toggle;
}

void CAT_graph_draw(float* values, uint16_t* colours, uint16_t count)
{
	float wdw_w = wdw_x1 - wdw_x0 + 1;
	float wdw_h = wdw_y1 - wdw_y0 + 1;

	if(bg_colour != CAT_TRANSPARENT)
	{
		CAT_fillberry(wdw_x0, wdw_y0, wdw_w, wdw_h, bg_colour);
	}

	if(auto_viewport)
	{
		vp_y0 = INFINITY;
		vp_y1 = -INFINITY;
		for (int i = 0; i < count; i++)
		{
			float val = values[i];
			vp_y0 = min(vp_y0, val);
			vp_y1 = max(vp_y1, val);
		}
	}
	float range = (vp_y1-vp_y0);
	float mid_range = (vp_y0+vp_y1)/2;

	float mid_y = (wdw_y0+wdw_y1)/2;
	float y_scale = range > 0 ? -wdw_h / range : 0;
	float x_scale = wdw_w / (float) (count-1);
	x_scale = max(x_scale, 1);

	CAT_CSCLIP_set_rect(wdw_x0, wdw_y0-1, wdw_x1, wdw_y1+1);
	float x0 = wdw_x0;
	float y0 = mid_y + (values[0]-mid_range) * y_scale;
	for(int i = 1; i < count; i++)
	{
		float x1 = x0 + x_scale;
		float y1 = mid_y + (values[i]-mid_range) * y_scale;
		
		int x0i = x0; int y0i = y0; int x1i = x1; int y1i = y1;
		if(CAT_CSCLIP(&x0i, &y0i, &x1i, &y1i))
		{
			CAT_lineberry(x0i, y0i, x1i, y1i, fg_colour);
			if(pt_size > 0)
			{
				uint16_t c0 = colours == NULL ? fg_colour : colours[i-1];
				uint16_t c1 = colours == NULL ? fg_colour : colours[i];
				if(pt_fill)
				{
					CAT_discberry(x0i, y0i, pt_size, c0);
					CAT_discberry(x1i, y1i, pt_size, c1);
				}
				else
				{
					CAT_circberry(x0i, y0i, pt_size, c0);
					CAT_circberry(x1i, y1i, pt_size, c1);
				}
			}
		}

		x0 = x1;
		y0 = y1;
	}
}