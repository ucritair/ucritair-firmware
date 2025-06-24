#pragma once

#include "cat_render.h"

void CAT_draw_arrows(int x, int y, int size, int dist, uint16_t c);
void CAT_draw_cross_box(int x0, int y0, int x1, int y1, uint16_t c);
void CAT_draw_corner_box(int x0, int y0, int x1, int y1, uint16_t c);
void CAT_draw_progress_bar(int x, int y, int w, int h, uint16_t co, uint16_t ci, float t);
