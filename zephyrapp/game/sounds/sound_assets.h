// Generated from /Users/tomas/Documents/GitHub/cat_software/zephyrapp/game/build/../sounds/sounds.json

#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct CAT_sound
{
	const char* path;
	uint8_t* samples;
	size_t size;
} CAT_sound;

extern CAT_sound coin_sound;
extern CAT_sound fail_sound;
extern CAT_sound thud_sound;
