#pragma once

#include "cat_render.h"

typedef struct
{
	struct scene_single
	{
		uint8_t idx;
		uint16_t x, y;
	}* singles;
	uint8_t single_count;

	struct scene_region
	{
		uint8_t idx;
		uint16_t x0, y0, x1, y1;
	}* regions;
	uint8_t region_count;
} CAT_scene_layer;

typedef struct
{
	const char* name;
	
	const CAT_sprite** palette;
	uint8_t palette_length;

	CAT_scene_layer* layers;
	uint8_t layer_count;

	struct
	{
		uint16_t x, y, w, h;
	} skybox;
} CAT_scene;