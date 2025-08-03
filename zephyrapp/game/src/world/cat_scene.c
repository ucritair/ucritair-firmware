#include "cat_scene.h"
#include "cat_gui.h"
#include "cat_math.h"

void CAT_scene_get_position(const CAT_scene* scene, CAT_scene_index* index, CAT_scene_point out)
{
	struct layer* layer = &scene->layers[index->layer];
	struct prop* prop = &layer->props[index->prop];
	out[0] = prop->position_x;
	out[1] = prop->position_y;
}

void CAT_scene_get_AABB(const CAT_scene* scene, CAT_scene_index* index, CAT_scene_AABB out)
{
	struct layer* layer = &scene->layers[index->layer];
	struct prop* prop = &layer->props[index->prop];
	switch(index->leaf)
	{
		case BLOCKER:
		{
			out[0] = prop->prop->blockers[index->blocker][0] + prop->position_x;
			out[1] = prop->prop->blockers[index->blocker][1] + prop->position_y;
			out[2] = prop->prop->blockers[index->blocker][2] + prop->position_x;
			out[3] = prop->prop->blockers[index->blocker][3] + prop->position_y;
		}
		break;

		case TRIGGER:
		{
			out[0] = prop->prop->triggers[index->trigger].aabb[0] + prop->position_x;
			out[1] = prop->prop->triggers[index->trigger].aabb[1] + prop->position_y;
			out[2] = prop->prop->triggers[index->trigger].aabb[2] + prop->position_x;
			out[3] = prop->prop->triggers[index->trigger].aabb[3] + prop->position_y;
		}
		break;
	}
}

void CAT_scene_get_direction(const CAT_scene* scene, CAT_scene_index* index, CAT_scene_vector out)
{
	if(index->leaf != TRIGGER)
		return;
	struct layer* layer = &scene->layers[index->layer];
	struct prop* prop = &layer->props[index->prop];
	out[0] = prop->prop->triggers[index->trigger].tx;
	out[1] = prop->prop->triggers[index->trigger].ty;
}

CAT_scene_index collision_buffer[256];
uint8_t collision_head = 0;

void clear_collisions()
{
	collision_head = 0;
}

void push_collision(int layer_idx, int prop_idx, int blocker_idx, int trigger_idx)
{
	if(collision_head == 255)
		return;

	collision_buffer[collision_head] = (const CAT_scene_index) {
		.leaf = blocker_idx != -1 ? BLOCKER : TRIGGER,
		.prop = prop_idx,
		.blocker = blocker_idx,
		.trigger = trigger_idx,
	};
	collision_head += 1;
}

CAT_scene_index* CAT_detect_collisions(const CAT_scene* scene, int x0, int y0, int x1, int y1, int* count)
{
	clear_collisions();

	for(int i = 0; i < scene->layer_count; i++)
	{
		struct layer* layer = &scene->layers[i];
		for(int j = 0; j < layer->prop_count; j++)
		{
			struct prop* prop = &layer->props[j];

			for(int k = 0; k < prop->prop->blocker_count; k++)
			{
				int bx0 = prop->prop->blockers[k][0] + prop->position_x;
				int by0 = prop->prop->blockers[k][1] + prop->position_y;
				int bx1 = prop->prop->blockers[k][2] + prop->position_x;
				int by1 = prop->prop->blockers[k][3] + prop->position_y;

				bool collision = 
				CAT_rect_rect_touching
				(
					x0, y0, x1, y1,
					bx0, by0, bx1, by1
				);
				if(collision)
					push_collision(i, j, k, -1);
			}

			for(int k = 0; k < prop->prop->trigger_count; k++)
			{
				int tx0 = prop->prop->triggers[k].aabb[0] + prop->position_x;
				int ty0 = prop->prop->triggers[k].aabb[1] + prop->position_y;
				int tx1 = prop->prop->triggers[k].aabb[2] + prop->position_x;
				int ty1 = prop->prop->triggers[k].aabb[3] + prop->position_y;

				bool collision = 
				CAT_rect_rect_touching
				(
					x0, y0, x1, y1,
					tx0, ty0, tx1, ty1
				);
				if(collision)
					push_collision(i, j, -1, k);
			}
		}
	}

	*count = collision_head;
	return collision_buffer;
}
