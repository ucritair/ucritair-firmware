// Generated from sprites/sprites.json

#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct CAT_sprite2
{
	const char* path;

	uint16_t* colour_table;
	uint8_t n_colours;

	uint8_t* runs;
	uint16_t n_runs;

	uint16_t width;
	uint16_t height;
	uint8_t frames;
} CAT_sprite2;

extern CAT_sprite2 none_24x24_sprite;
