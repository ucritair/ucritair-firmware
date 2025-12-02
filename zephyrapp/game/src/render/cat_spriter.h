#pragma once

#include "cat_render.h"

bool CAT_tinysprite_read(const CAT_tinysprite* sprite, int x, int y);
void CAT_draw_tinysprite(int x, int y, const CAT_tinysprite* sprite, uint16_t fg, uint16_t bg);
void CAT_draw_tinyglyph(int x, int y, char g, uint16_t c);
void CAT_draw_tinystring(int x, int y, const char* s, uint16_t c);