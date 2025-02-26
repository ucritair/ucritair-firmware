#include "menu_graph.h"
#include "cat_render.h"

void plot_px(int x, int y, uint16_t color)
{
	if (y < 0 || y > LCD_FRAMEBUFFER_H) return;
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	framebuffer[(y*LCD_IMAGE_W) + x] = color;
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
	yoff -= FRAMEBUFFER_ROW_OFFSET;
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
			int y0 = yoff+FRAMEBUFFER_ROW_OFFSET+GRAPH_H-v0;
			int y1 = yoff+FRAMEBUFFER_ROW_OFFSET+GRAPH_H-v1;
			CAT_lineberry(xoff+x, y0, xoff+x+1, y1, 0x001F);
		}
	}

	if(cursor_start >= 0 && cursor_start < GRAPH_W)
		CAT_lineberry(xoff+cursor_start, yoff, xoff+cursor_start, yoff+FRAMEBUFFER_ROW_OFFSET+GRAPH_H-1, 0x07E0);
	if(cursor_end >= 0 && cursor_end < GRAPH_W)
		CAT_lineberry(xoff+cursor_end, yoff, xoff+cursor_end, yoff+FRAMEBUFFER_ROW_OFFSET+GRAPH_H-1, 0xF800);
}
