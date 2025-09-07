#pragma once

#include "cat_render.h"

void CAT_draw_arrows(int x, int y, int size, int dist, uint16_t c);
void CAT_draw_cross_box(int x0, int y0, int x1, int y1, uint16_t c);
void CAT_draw_corner_box(int x0, int y0, int x1, int y1, uint16_t c);
void CAT_draw_progress_bar(int x, int y, int w, int h, uint16_t co, uint16_t ci, float t);
void CAT_draw_dot_grid(int x, int y, int w, int h, int s, uint16_t c);

typedef enum
{
	CAT_GIZMO_PRIMITIVE_RING,
	CAT_GIZMO_PRIMITIVE_TRI,
	CAT_GIZMO_PRIMITIVE_BOX,
	CAT_GIZMO_PRIMITIVE_HEX
} CAT_gizmo_primitive;

void CAT_draw_regular_polygon(int n, int x, int y, int r, float t, uint16_t c);
void CAT_draw_gizmo_primitive(CAT_gizmo_primitive primitive, int x, int y, int r, float t, uint16_t c);

void CAT_draw_page_markers(int y, int pages, int page, uint16_t c);
void CAT_draw_page_alert(int y, int pages, int page, uint16_t c);
void CAT_draw_subpage_markers(int y, int pages, int page, uint16_t c);
void CAT_draw_delta_arrow(int x, int y, bool up, uint16_t c);