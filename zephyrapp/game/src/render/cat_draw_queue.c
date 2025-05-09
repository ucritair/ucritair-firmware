#include "cat_render.h"

#include "sprite_assets.h"

//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

static CAT_draw_job jobs[CAT_DRAW_QUEUE_MAX_LENGTH];
static int job_count = 0;

void CAT_draw_queue_clear()
{
	job_count = 0;
}

void CAT_draw_queue_insert(int idx, const CAT_sprite* sprite, int frame_idx, int layer, int x, int y, int flags)
{
	if(job_count >= CAT_DRAW_QUEUE_MAX_LENGTH)
	{
		CAT_printf("[WARNING] Attempted add to full draw queue\n");
		return;
	}

	for(int i = job_count; i > idx; i--)
	{
		jobs[i] = jobs[i-1];
	}
	jobs[idx] = (CAT_draw_job) {sprite, frame_idx, layer, x, y, flags};
	job_count += 1;
}

int CAT_draw_queue_add(const CAT_sprite* sprite, int frame_idx, int layer, int x, int y, int flags)
{
	if(CAT_is_first_render_cycle())
	{
		int insert_idx = job_count;
		for(int i = 0; i < insert_idx; i++)
		{
			CAT_draw_job other = jobs[i];
			if(layer < other.layer || (layer == other.layer && y < other.y))
			{
				insert_idx = i;
				break;
			}
		}

		CAT_draw_queue_insert(insert_idx, sprite, frame_idx, layer, x, y, flags);
		return insert_idx;
	}
	return -1;
}

void CAT_draw_queue_submit()
{
	for(int i = 0; i < job_count; i++)
	{
		CAT_draw_job* job = &jobs[i];
		const CAT_sprite* sprite = job->sprite;

		if(job->frame_idx == -1)
			job->frame_idx = CAT_animator_get_frame(sprite);

		CAT_push_draw_flags(job->flags);
		CAT_draw_sprite(sprite, job->frame_idx, job->x, job->y);
	}
}