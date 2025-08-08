#pragma once

#include "cat_render.h"

typedef int16_t CAT_scene_vector[2];
typedef CAT_scene_vector CAT_scene_point;
typedef int16_t CAT_scene_AABB[4];

typedef struct
{
	const CAT_sprite* sprite;
	bool palette;

	int16_t** blockers;
	uint8_t blocker_count;
	
	struct trigger
	{
		int16_t aabb[4];
		int8_t tx, ty;
		void (*proc) ();
	}* triggers;
	uint8_t trigger_count;
} CAT_prop;

typedef struct
{
	struct layer
	{
		struct prop
		{
			const CAT_prop* prop;
			int16_t position_x, position_y;
			int16_t variant;
		}* props;
		uint16_t prop_count;
	}* layers;
	uint8_t layer_count;
} CAT_scene;

typedef struct
{
	enum {BLOCKER, TRIGGER} leaf;
	uint8_t layer;
	uint16_t prop;
	uint8_t blocker;
	uint8_t trigger;
} CAT_scene_index;

void CAT_scene_get_position(const CAT_scene* scene, CAT_scene_index* index, CAT_scene_point out);
bool CAT_scene_get_AABB(const CAT_scene* scene, CAT_scene_index* index, CAT_scene_AABB out);
void CAT_scene_get_direction(const CAT_scene* scene, CAT_scene_index* index, CAT_scene_vector out);
bool CAT_scene_get_prop_asset(const CAT_scene* scene, CAT_scene_index* index, const CAT_prop** out);

CAT_scene_index* CAT_detect_collisions(const CAT_scene* scene, int x0, int y0, int x1, int y1, int* count);
