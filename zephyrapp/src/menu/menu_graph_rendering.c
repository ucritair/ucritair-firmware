#include "menu_graph.h"
#include "cat_sprite.h"

void plot_px(int x, int y, uint16_t color)
{
	if (y < 0 || y > LCD_FRAMEBUFFER_H) return;

	FRAMEBUFFER[(y*LCD_IMAGE_W) + x] = color;
}

void fill_rect(int x0, int y0, int w, int h, uint16_t color)
{
	for (int dx = 0; dx < w; dx++)
	{
		for (int dy = 0; dy < h; dy++)
		{
			plot_px(x0+dx, y0+dy, color);
		}
	}
}

void CAT_do_render_graph(int16_t* data, int max, int xoff, int yoff, int cursor_start, int cursor_end)
{
	yoff -= framebuffer_offset_h;
	if (yoff > LCD_FRAMEBUFFER_H || (yoff+GRAPH_H) < 0) return;

	fill_rect(xoff, yoff, GRAPH_W+GRAPH_MARGIN+GRAPH_MARGIN, GRAPH_MARGIN, 0);
	fill_rect(xoff, yoff, GRAPH_MARGIN, GRAPH_H+GRAPH_MARGIN+GRAPH_MARGIN, 0);
	fill_rect(xoff+GRAPH_W+GRAPH_MARGIN, yoff, GRAPH_MARGIN, GRAPH_H+GRAPH_MARGIN+GRAPH_MARGIN, 0);
	fill_rect(xoff, yoff+GRAPH_MARGIN+GRAPH_H, GRAPH_W+GRAPH_MARGIN+GRAPH_MARGIN, GRAPH_MARGIN, 0);

	xoff += GRAPH_MARGIN;
	yoff += GRAPH_MARGIN;
	
	for(int x = 0; x < GRAPH_W-1; x++)
	{	
		int v0 = data[x];
		int v1 = data[x+1];
		if(v0 >= 0 && v1 >= 0)
		{
			int y0 = yoff+framebuffer_offset_h+GRAPH_H-v0;
			int y1 = yoff+framebuffer_offset_h+GRAPH_H-v1;
			CAT_bresenham(xoff+x, y0, xoff+x+1, y1, 0xF000);
		}
	}

	if(cursor_start >= 0 && cursor_start < GRAPH_W)
		CAT_bresenham(xoff+cursor_start, yoff, xoff+cursor_start, yoff+framebuffer_offset_h+GRAPH_H-1, 0x000F);
	if(cursor_end >= 0 && cursor_end < GRAPH_W)
		CAT_bresenham(xoff+cursor_end, yoff, xoff+cursor_end, yoff+framebuffer_offset_h+GRAPH_H-1, 0x00F0);

	/*int last_rendered_h = -1;

	for (int x = 0; x < GRAPH_W; x++)
	{
		int h = data[x];
		
		if (x==cursor_start)
		{
			for (int i = 0; i < GRAPH_H; i++)
			{
				plot_px(xoff+x, yoff+i, 0x000f);
			}
		}

		if (x==cursor_end)
		{
			for (int i = 0; i < GRAPH_H; i++)
			{
				plot_px(xoff+x, yoff+i, 0x00f0);
			}
		}

		if (h == -1)
		{
			continue;
		}

		while (h--)
		{
			plot_px(xoff+x, yoff + GRAPH_H - h, 0xf000);
		}
	}*/
}
