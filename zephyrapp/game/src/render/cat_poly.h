#pragma once

#include <stdint.h>

#define CAT_POLY_VERTEX_COUNT(buffer) ((sizeof(buffer)/sizeof(buffer[0]))/2)

typedef struct
{
	float* verts;
	uint16_t n_verts;
	uint16_t* faces;
	uint16_t n_faces;
} CAT_mesh;

typedef struct
{
	uint16_t* verts;
	uint16_t vert_count;
	uint16_t* edges;
	uint16_t edge_count;
} CAT_mesh2d;

void CAT_draw_mesh2d(const CAT_mesh2d* mesh, int x, int y, uint16_t c);

void CAT_poly_set_scale(float x, float y);
void CAT_poly_set_rotation(float t);
void CAT_poly_set_translation(float x, float y);
void CAT_poly_clear_transformation();

void CAT_poly_draw(float* vertices, uint16_t vertex_count, uint16_t c);