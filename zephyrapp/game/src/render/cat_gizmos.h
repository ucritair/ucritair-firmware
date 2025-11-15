#pragma once

#include <stdint.h>
#include "cat_math.h"

void CAT_draw_regular_polygon(int n, int x, int y, int r, float t, uint16_t c);
void CAT_draw_arrow(int x, int y, int w, int h, CAT_orientation d, uint16_t c);
void CAT_draw_cross(int x, int y, int w, int h, uint16_t c);

void CAT_draw_arrows(int x, int y, int size, int dist, uint16_t c);
void CAT_draw_cross_box(int x0, int y0, int x1, int y1, uint16_t c);
void CAT_draw_corner_box(int x0, int y0, int x1, int y1, uint16_t c);
void CAT_draw_progress_bar(int x, int y, int w, int h, uint16_t co, uint16_t ci, float t);
void CAT_draw_dot_grid(int x, int y, int w, int h, int s, uint16_t c);
void CAT_draw_score_line(int x, int y, int w, float t, float dt, uint16_t c, uint16_t ct, uint16_t cd);

void CAT_draw_page_markers(int y, int pages, int page, uint16_t c);
void CAT_draw_page_alert(int y, int pages, int page, uint16_t c);
void CAT_draw_subpage_markers(int y, int pages, int page, uint16_t c);

void CAT_draw_arrow_slider(int x0, int y0, int x1, int y1, float t, uint16_t c);
void CAT_draw_empty_sfx(int x0, int y0, int a, int b, uint16_t c);

void CAT_draw_lock(int x, int y, int r, float t, uint16_t c);

void CAT_draw_dpad(int x, int y, int r, int mask, uint16_t cf, uint16_t cb);
void CAT_draw_star(int x, int y, int r, uint16_t c);

void CAT_draw_button(int x, int y, int button, uint16_t c);