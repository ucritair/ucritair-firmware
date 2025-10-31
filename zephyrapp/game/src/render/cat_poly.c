#include "cat_poly.h"
#include "cat_render.h"
#include "cat_math.h"

void CAT_draw_mesh2d(const CAT_mesh2d* mesh, int x, int y, uint16_t c)
{
	for(int i = 0; i < mesh->edge_count; i++)
	{
		uint16_t v0_idx = mesh->edges[i*2+0];
		uint16_t v1_idx = mesh->edges[i*2+1];
		uint8_t v0_x = mesh->verts[v0_idx*2+0];
		uint8_t v0_y = mesh->verts[v0_idx*2+1];
		uint8_t v1_x = mesh->verts[v1_idx*2+0];
		uint8_t v1_y = mesh->verts[v1_idx*2+1];
		CAT_lineberry(v0_x+x, v0_y+y, v1_x+x, v1_y+y, c);
	}
}

static float scale_x = 1.0f;
static float scale_y = 1.0f;
static float rotation = 0.0f;
static float costheta = 1.0f;
static float sintheta = 0.0f;
static float translation_x = 0.0f;
static float translation_y = 0.0f;

void CAT_poly_set_scale(float x, float y)
{
	scale_x = x;
	scale_y = y;
}

void CAT_poly_set_rotation(float t)
{
	rotation = t;
	costheta = CAT_cos(t);
	sintheta = CAT_sin(t);
}

void CAT_poly_set_translation(float x, float y)
{
	translation_x = x;
	translation_y = y;
}

void CAT_poly_clear_transformation()
{
	scale_x = 1.0f;
	scale_y = 1.0f;
	rotation = 0.0f;
	costheta = 1.0f;
	sintheta = 0.0f;
	translation_x = 0.0f;
	translation_y = 0.0f;
}

void CAT_poly_draw(float* vertices, uint16_t vertex_count, uint16_t c)
{
	for(int i = 0; i < vertex_count-1; i += 2)
	{
		float x0 = vertices[i*2+0];
		float y0 = vertices[i*2+1];
		float x1 = vertices[i*2+2];
		float y1 = vertices[i*2+3];

		x0 *= scale_x;
		y0 *= scale_y;
		x1 *= scale_x;
		y1 *= scale_y;

		float x = x0, y = y0;
		x0 = costheta * x - sintheta * y;
		y0 = sintheta * x + costheta * y;
		x = x1; y = y1;
		x1 = costheta * x - sintheta * y;
		y1 = sintheta * x + costheta * y;
		y0 *= -1; y1 *= -1;

		x0 += translation_x;
		y0 += translation_y;
		x1 += translation_x;
		y1 += translation_y;

		CAT_CSCLIP_set_rect(0, 0, CAT_LCD_SCREEN_W-1, CAT_LCD_SCREEN_H-1);
		int x0i = x0, y0i = y0, x1i = x1, y1i = y1; 
		if(CAT_CSCLIP(&x0i, &y0i, &x1i, &y1i))
		{
			CAT_lineberry(x0i, y0i, x1i, y1i, c);
		}
	}
}