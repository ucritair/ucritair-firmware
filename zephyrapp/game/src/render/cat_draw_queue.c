#include "cat_render.h"

//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

CAT_anim_table anim_table;

void CAT_anim_table_init()
{
	for(int i = 0; i < CAT_ANIM_TABLE_MAX_LENGTH; i++)
	{
		anim_table.frame_idx[i] = 0;
		anim_table.loop[i] = true;
		anim_table.reverse[i] = false;
		anim_table.dirty[i] = true;
	}
}

void CAT_anim_toggle_loop(const CAT_sprite* sprite, bool toggle)
{
	if(sprite == NULL)
	{
		CAT_printf("[ERROR] CAT_anim_toggle_loop: null sprite\n");
		return;
	}

	anim_table.loop[sprite->id] = toggle;
}

void CAT_anim_toggle_reverse(const CAT_sprite* sprite, bool toggle)
{
	if(sprite == NULL)
	{
		CAT_printf("[ERROR] CAT_anim_toggle_reverse: null sprite\n");
		return;
	}

	anim_table.reverse[sprite->id] = toggle;
}

bool CAT_anim_finished(const CAT_sprite* sprite)
{
	if(sprite == NULL)
	{
		CAT_printf("[ERROR] CAT_anim_finished: null sprite\n");
		return true;
	}

	return anim_table.frame_idx[sprite->id] == sprite->frame_count-1;
}

void CAT_anim_reset(const CAT_sprite* sprite)
{
	if(sprite == NULL)
	{
		CAT_printf("[ERROR] CAT_anim_reset: null sprite\n");
		return;
	}

	anim_table.frame_idx[sprite->id] = 0;
}

static int counter = 0;
bool CAT_anim_should_tick()
{
	counter += 1;
	bool result = counter >= 2;
	if(result)
		counter = 0;
	return result;
}

static CAT_draw_job jobs[CAT_DRAW_QUEUE_MAX_LENGTH];
static int job_count = 0;

void CAT_draw_queue_clear()
{
	job_count = 0;
}

void CAT_draw_queue_insert(int idx, const CAT_sprite* sprite, int frame_idx, int layer, int x, int y, int mode)
{
	if(sprite == NULL)
	{
		CAT_printf("[ERROR] CAT_draw_queue_insert: null sprite\n");
		return;
	}
	if(job_count >= CAT_DRAW_QUEUE_MAX_LENGTH)
	{
		CAT_printf("[WARNING] Attempted add to full draw queue\n");
		return;
	}

	for(int i = job_count; i > idx; i--)
	{
		jobs[i] = jobs[i-1];
	}
	jobs[idx] = (CAT_draw_job) {sprite, frame_idx, layer, x, y, mode};
	job_count += 1;
}

int CAT_draw_queue_add(const CAT_sprite* sprite, int frame_idx, int layer, int x, int y, int mode)
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

		CAT_draw_queue_insert(insert_idx, sprite, frame_idx, layer, x, y, mode);
		return insert_idx;
	}
	return -1;
}

void CAT_draw_queue_submit()
{
	if (CAT_is_first_render_cycle())
	{
		if(CAT_anim_should_tick())
		{
			for(int i = 0; i < job_count; i++)
			{
				CAT_draw_job* job = &jobs[i];
				if(job->frame_idx != -1)
					continue;
				if(!anim_table.dirty[job->sprite->id])
					continue;
			
				if(anim_table.frame_idx[job->sprite->id] < job->sprite->frame_count-1)
					anim_table.frame_idx[job->sprite->id] += 1;
				else if(anim_table.loop[job->sprite->id])
					anim_table.frame_idx[job->sprite->id] = 0;
				
				anim_table.dirty[job->sprite->id] = false;
			}
		}

		for(int i = 0; i < job_count; i++)
		{
			CAT_draw_job* job = &jobs[i];
			anim_table.dirty[job->sprite->id] = true;
		}
	}

	for(int i = 0; i < job_count; i++)
	{
		CAT_draw_job* job = &jobs[i];
		const CAT_sprite* sprite = job->sprite;
		if(job->frame_idx == -1)
		{
			job->frame_idx = anim_table.frame_idx[sprite->id];
			if(anim_table.reverse[sprite->id])
				job->frame_idx = sprite->frame_count-1-job->frame_idx;
		}
		draw_mode = job->mode;
		CAT_draw_sprite(sprite, job->frame_idx, job->x, job->y);
	}
}