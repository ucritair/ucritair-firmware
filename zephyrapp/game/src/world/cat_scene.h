#pragma once

#include "cat_render.h"

typedef struct
{
	const CAT_sprite* sprite;

	int16_t** blockers;
	uint8_t blocker_count;
	
	struct trigger
	{
		int16_t aabb[4];
		void (*proc) ();
	}* triggers;
	uint8_t trigger_count;
} CAT_prop;