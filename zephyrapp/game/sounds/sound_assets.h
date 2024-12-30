#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct CAT_sound
{
	uint8_t* samples;
	size_t size;
} CAT_sound;

extern CAT_sound coin_sound;
extern CAT_sound fail_sound;
extern CAT_sound thud_sound;
