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

typedef struct
{
	int16_t origin_x, origin_y;
	
	struct layer
	{
		struct prop
		{
			const CAT_prop* prop;
			int16_t position_x, position_y;
		}* props;
		uint8_t prop_count;
	}* layers;
	uint8_t layer_count;
} CAT_scene;