#pragma once

#include <stdint.h>
#include <stdbool.h>

void CAT_graph_set_window(int x0, int y0, int x1, int y1);
void CAT_graph_set_viewport(float x0, float y0, float x1, float y1);
void CAT_graph_set_auto_viewport(bool toggle);

void CAT_graph_set_background(uint16_t colour);
void CAT_graph_set_foreground(uint16_t colour);

void CAT_graph_set_point_size(uint8_t size);
void CAT_graph_set_point_fill(bool toggle);

void CAT_graph_draw(float* values, uint16_t* colours, uint16_t count);