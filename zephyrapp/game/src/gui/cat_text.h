#pragma once

#define CAT_TEXT_RAW_BUFFER_CAPACITY 1024
#define CAT_TEXT_SKIP_BUFFER_CAPACITY 32
#define CAT_TEXT_BREAK_BUFFER_CAPACITY 64
#define CAT_TEXT_COLOUR_BUFFER_CAPACITY 16

void CAT_draw_text2(int x, int y, const char* text);
void CAT_draw_textf2(int x, int y, const char* fmt, ...);