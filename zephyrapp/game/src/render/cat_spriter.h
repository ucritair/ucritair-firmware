#pragma once

#include "cat_render.h"

bool CAT_tinysprite_read(const CAT_tinysprite* sprite, int x, int y);
void CAT_draw_tinysprite(int x, int y, const CAT_tinysprite* sprite, uint16_t fg, uint16_t bg);
void CAT_draw_tinyglyph(int x, int y, char g, uint16_t c);
void CAT_draw_tinystring(int x, int y, const char* s, uint16_t c);

void CAT_eink_write_pixel(int x, int y, bool value);
void CAT_eink_draw_sprite(int x, int y, const CAT_tinysprite* sprite);
void CAT_eink_draw_glyph(int x, int y, int scale, char g);
void CAT_eink_draw_string(int x, int y, int scale, const char* s);