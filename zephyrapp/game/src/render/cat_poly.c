#include "cat_render.h"

void CAT_draw_mesh2d(const CAT_mesh2d* mesh, int x, int y, uint16_t c)
{
	for(int i = 0; i < mesh->edge_count; i++)
	{
		uint8_t v0_idx = mesh->edges[i*2+0];
		uint8_t v1_idx = mesh->edges[i*2+1];
		uint8_t v0_x = mesh->verts[v0_idx*2+0];
		uint8_t v0_y = mesh->verts[v0_idx*2+1];
		uint8_t v1_x = mesh->verts[v1_idx*2+0];
		uint8_t v1_y = mesh->verts[v1_idx*2+1];
		CAT_lineberry(v0_x+x, v0_y+y, v1_x+x, v1_y+y, c);
	}
}

void CAT_draw_polyline(int x, int y, int16_t* poly, int count, uint16_t c, CAT_poly_mode mode)
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