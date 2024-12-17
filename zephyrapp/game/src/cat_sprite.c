#include "cat_sprite.h"
#include "../sprites/sprite_assets.h"

#include <stdint.h>
#include <string.h>
#include "cat_core.h"
#include <stdint.h>
#include "cat_math.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef CAT_DESKTOP
#define FRAMEBUFFER spriter.framebuffer
#define LCD_FRAMEBUFFER_H LCD_SCREEN_H
#else
#include "lcd_driver.h"
#define FRAMEBUFFER lcd_framebuffer
#endif

//////////////////////////////////////////////////////////////////////////
// THE BERRIER

uint8_t luminance(uint16_t rgb)
{
	uint8_t r = ((rgb & 0b1111100000000000) >> 11);
	uint8_t g = (rgb & 0b0000011111100000) >> 5;
	uint8_t b = rgb & 0b0000000000011111;
	uint8_t l = ((r << 1) + r  + (g << 2) + b) >> 1;
	return l;
}

// CATs when they eat a
void CAT_greenberry(int xi, int w, int yi, int h, float t)
{
#ifdef CAT_EMBEDDED
	yi -= framebuffer_offset_h;
#endif

	int xf = xi + w * t;
	int yf = yi + h;

#ifdef CAT_EMBEDDED
	if (yi > LCD_FRAMEBUFFER_H || yf < 0)
		return;

	if (yf >= LCD_FRAMEBUFFER_H)
		yf = LCD_FRAMEBUFFER_H-1;

	if (yi < 0)
		yi = 0;
#endif

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			int idx = y * LCD_SCREEN_W + x;
#ifdef CAT_DESKTOP
			uint16_t c = FRAMEBUFFER[idx];
			uint8_t l = luminance(c);
			FRAMEBUFFER[idx] = RGB8882565(l >> 1, l, l >> 1);
#else
			// r4 r3 r2 r1 r0 g5 g4 g3     g2 g1 g0 b4 b3 b2 b1 b0
			// g2 g1 g0 b4 b3 b2 b1 b0     r4 r3 r2 r1 r0 g5 g4 g3
			uint16_t px = FRAMEBUFFER[idx];

			px |= 0b011;
			px &= (0b00010000<<8) | 0b10001111;

			FRAMEBUFFER[idx] = px;
#endif
		}
	}
}
// Okay, it's more of an orangeberry. [Goldberry?](https://tolkiengateway.net/wiki/Goldberry)

void CAT_frameberry(uint16_t c)
{
#ifdef CAT_EMBEDDED
	c = (c >> 8) | ((c & 0xff) << 8);
#endif

	uint16_t* px = FRAMEBUFFER;
	uint16_t* end = FRAMEBUFFER + LCD_SCREEN_W * LCD_FRAMEBUFFER_H;
	while(px != end)
	{
		*(px++) = c;
	}
}


// Cats would probably never eat this one
void CAT_greyberry(int xi, int w, int yi, int h)
{
#ifdef CAT_EMBEDDED
	yi -= framebuffer_offset_h;
#endif

	int xf = xi + w;
	int yf = yi + h;

#ifdef CAT_EMBEDDED
	if (yi > LCD_FRAMEBUFFER_H || yf < 0)
		return;

	if (yf >= LCD_FRAMEBUFFER_H)
		yf = LCD_FRAMEBUFFER_H-1;

	if (yi < 0)
		yi = 0;
#endif

	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			int idx = y * LCD_SCREEN_W + x;
#ifdef CAT_DESKTOP
			uint16_t c = FRAMEBUFFER[idx];
			uint8_t l = luminance(c);
			FRAMEBUFFER[idx] = RGB8882565(l, l, l);
#else
			// r4 r3 r2 r1 r0 g5 g4 g3     g2 g1 g0 b4 b3 b2 b1 b0
			// g2 g1 g0 b4 b3 b2 b1 b0     r4 r3 r2 r1 r0 g5 g4 g3
			uint16_t px = FRAMEBUFFER[idx];
			px &= (0b00010000<<8) | 0b10000100;
			FRAMEBUFFER[idx] = px;
#endif
		}
	}
}

// implementation based on Dmitri Sokolov's
void CAT_lineberry(int xi, int yi, int xf, int yf, uint16_t c)
{
#ifdef CAT_EMBEDDED
	c = (c >> 8) | ((c & 0xff) << 8);
#endif

	// if the line is steep, transpose its start and end points
	bool steep = abs(yf-yi) > abs(xf-xi);
	if(steep)
	{
		int temp = xi;
		xi = yi;
		yi = temp;

		temp = xf;
		xf = yf;
		yf = temp;
	}

	// if the line heads left, swap its start and end points
	bool leftward = xi > xf;
	if(leftward)
	{
		int temp = xi;
		xi = xf;
		xf = temp;

		temp = yi;
		yi = yf;
		yf = temp;
	}
	
	int dx = xf - xi;
	int dy = yf - yi;

	// account for line heading up or down
	int y_step = (yf > yi) ? 1 : -1;
	int y = yi;
	
	// approximate d_err as abs(dy) / (dx ~= 0.5)
	int d_err = abs(dy) * 2;
	int err = 0;

	// if line is steep, we swap x,y in the draw call to undo our earlier transposition
	// we employ a branch between two for loops to avoid branching within one loop
	if(steep)
	{
		for(int x = xi; x < xf; x++)
		{
#ifdef CAT_EMBEDDED
			int xf = x - framebuffer_offset_h;
			if(y >= 0 && y < LCD_FRAMEBUFFER_W && xf >= 0 && xf < LCD_FRAMEBUFFER_H)
				FRAMEBUFFER[xf * LCD_FRAMEBUFFER_W + y] = c;
#else
			if(y >= 0 && y < LCD_SCREEN_W && x >= 0 && x < LCD_SCREEN_H)
				FRAMEBUFFER[x * LCD_SCREEN_W + y] = c;
#endif

			err += d_err;
			if(err > dx)
			{
				y += y_step;
				err -= dx*2;
			}
		}
	}
	else
	{
		for(int x = xi; x < xf; x++)
		{
#ifdef CAT_EMBEDDED
			int yf = y - framebuffer_offset_h;
			if(x >= 0 && x < LCD_FRAMEBUFFER_W && yf >= 0 && yf < LCD_FRAMEBUFFER_H)
				FRAMEBUFFER[yf * LCD_FRAMEBUFFER_W + x] = c;
#else
			if(x >= 0 && x < LCD_SCREEN_W && y >= 0 && y < LCD_SCREEN_H)
				FRAMEBUFFER[y * LCD_SCREEN_W + x] = c;
#endif

			err += d_err;
			if(err > dx)
			{
				y += y_step;
				err -= dx*2;
			}
		}
	}
}

void CAT_fillberry(int xi, int yi, int w, int h, uint16_t c)
{
#ifdef CAT_EMBEDDED
	c = (c >> 8) | ((c & 0xff) << 8);
#endif

	for(int y = yi; y < yi+h; y++)
	{
		for(int x = xi; x < xi+w; x++)
		{
			FRAMEBUFFER[y * LCD_SCREEN_W + x] = c;
		}
	}
}

void CAT_strokeberry(int xi, int yi, int w, int h, uint16_t c)
{
	CAT_lineberry(xi, yi, xi+w, yi, c);
	CAT_lineberry(xi+w, yi, xi+w, yi+h, c);
	CAT_lineberry(xi+w, yi+h, xi, yi+h, c);
	CAT_lineberry(xi, yi+h, xi, yi, c);
}


//////////////////////////////////////////////////////////////////////////
// ATLAS

#ifdef CAT_EMBEDDED
extern const CAT_baked_sprite image_data_table[];
uint16_t rle_work_region[160];
#endif

CAT_sprite* CAT_sprite_get(int sprite_id)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid sprite: %d\n", sprite_id);
		return NULL;
	}

	return &atlas.data[sprite_id];
}


//////////////////////////////////////////////////////////////////////////
// SPRITER

CAT_spriter spriter;

void CAT_spriter_init()
{
#ifdef CAT_DESKTOP
	spriter.framebuffer = CAT_malloc(sizeof(uint16_t) * LCD_SCREEN_W * LCD_SCREEN_H);
#endif
	spriter.mode = CAT_DRAW_MODE_DEFAULT;	
}

#ifdef CAT_EMBEDDED
struct {
	const uint8_t* ptr;
	const uint16_t* colortab;
	int width;
	int rle_count;
	uint16_t rle_word;
} rle_decode_state;

void init_rle_decode(const CAT_baked_sprite* sprite, int frame_idx, int width)
{
	rle_decode_state.ptr = sprite->frames[frame_idx];
	rle_decode_state.colortab = sprite->color_table;
	rle_decode_state.width = width;
	rle_decode_state.rle_count = 0;
}

void unpack_rle_row()
{
#define RLE_NEXT() *(rle_decode_state.ptr++)
	// printf("Unpack %d, width=%d\n", sprite_id, width);
	int unpacked = 0;
	while (unpacked < rle_decode_state.width)
	{
		if (rle_decode_state.rle_count)
		{
			rle_work_region[unpacked++] = rle_decode_state.rle_word;
			rle_decode_state.rle_count--;
			continue;
		}

		uint16_t word = RLE_NEXT();
		// printf("word %04x ->", word);

		if (word == 0xff)
		{
			rle_decode_state.rle_word = rle_decode_state.colortab[RLE_NEXT()];
			rle_decode_state.rle_count = RLE_NEXT();

			// printf("rep %04x cnt %04x\n", repeated, repeat_count);
		}
		else
		{
			rle_work_region[unpacked++] = rle_decode_state.colortab[word];
			// printf("direct\n");
		}
	}
}
#endif

void CAT_draw_sprite(int sprite_id, int frame_idx, int x, int y)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid sprite: %d of %d\n", sprite_id);
		return;
	}

	CAT_sprite sprite = atlas.data[sprite_id];
	int w = sprite.width;
	int h = sprite.height;

#ifdef CAT_EMBEDDED
	init_rle_decode(&image_data_table[sprite_id], frame_idx, sprite.width);
#else
	const uint16_t* frame = &sprite.pixels[frame_idx * h * w];
#endif
	
	if((spriter.mode & CAT_DRAW_MODE_CENTER_X) > 0)
		x -= w/2;
	if((spriter.mode & CAT_DRAW_MODE_CENTER_Y) > 0)
		y -= h/2;
	else if((spriter.mode & CAT_DRAW_MODE_BOTTOM) > 0)
		y -= h;

#ifdef CAT_EMBEDDED
	y -= framebuffer_offset_h;

	if (y >= LCD_FRAMEBUFFER_H) return;
	if ((y + h) < 0) return;
#endif

	int y_end = y + h;
	if (y_end >= LCD_FRAMEBUFFER_H)
		y_end = LCD_FRAMEBUFFER_H;
	
	int dy = 0; // only on PC
	int inc_dir = (spriter.mode & CAT_DRAW_MODE_REFLECT_X)?-1:1;
	int initial_offset = inc_dir==1?0:w-1;

	while (y < y_end)
	{
#ifdef CAT_EMBEDDED
		unpack_rle_row();
#endif
		
		if(y < 0)
		{
			y++;
			dy++;
			continue;
		}

#ifdef CAT_EMBEDDED
		const uint16_t* read_ptr = &rle_work_region[initial_offset];
#else
		const uint16_t* read_ptr = &frame[dy * w + initial_offset];
#endif

		uint16_t* write_ptr = &FRAMEBUFFER[y * LCD_SCREEN_W + x];

		for(int dx = 0; dx < w; dx++)
		{
			int x_w = x+dx;

			if (x_w >= LCD_SCREEN_W)
				break;

			if(x_w >= 0)
			{
				uint16_t px = *read_ptr;

				if(px != 0xdead)
					*write_ptr = px;
			}

			read_ptr += inc_dir;
			write_ptr += 1;
		}

		y++;
		dy++;
	}
}

void CAT_draw_tiles(int sprite_id, int frame_idx, int y_t, int h_t)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid sprite: %d\n", sprite_id);
		return;
	}

	CAT_sprite sprite = atlas.data[sprite_id];

#ifndef CAT_EMBEDDED
	const uint16_t* frame = &sprite.pixels[frame_idx * CAT_TILE_SIZE * CAT_TILE_SIZE];
#endif

	int y_start = y_t * CAT_TILE_SIZE;
	int y_end = (y_t+h_t) * CAT_TILE_SIZE;

#ifdef CAT_EMBEDDED
	y_start -= framebuffer_offset_h;
	y_end -= framebuffer_offset_h;

	if (y_start >= LCD_FRAMEBUFFER_H) return;
	if (y_end < 0) return;

	if (y_start < 0) y_start = 0;
	if (y_end > LCD_FRAMEBUFFER_H) y_end = LCD_FRAMEBUFFER_H;
#endif

#ifdef CAT_EMBEDDED
	init_rle_decode(&image_data_table[sprite_id], frame_idx, sprite.width);
#endif

#if CAT_TILE_SIZE != 16
#error adjust tiler (rep count)
#endif

#ifdef CAT_EMBEDDED
#define RESETPTR from = (uint32_t*)rle_work_region;
#else
#define RESETPTR from = (uint32_t*)&frame[dy * CAT_TILE_SIZE];
#endif

#if (LCD_SCREEN_W/CAT_TILE_SIZE) != 15
#error adjust tiler (rep count)
#endif

#ifdef CAT_EMBEDDED
#if ((LCD_SCREEN_H/LCD_FRAMEBUFFER_SEGMENTS) % 16) != 0
#error adjust tiler (start pos)
#endif
#endif

#define UNROLL2PX *(to++) = *(from++);
#define UNROLL4PX UNROLL2PX UNROLL2PX
#define UNROLL8PX UNROLL4PX UNROLL4PX
#define UNROLL1TI RESETPTR UNROLL8PX UNROLL8PX
#define UNROLL2TI UNROLL1TI UNROLL1TI
#define UNROLL4TI UNROLL2TI UNROLL2TI
#define UNROLL8TI UNROLL4TI UNROLL4TI
#define UNROLL1LI UNROLL8TI UNROLL4TI UNROLL2TI UNROLL1TI

	for (int dy = 0; dy < CAT_TILE_SIZE; dy++)
	{
#ifdef CAT_EMBEDDED
		unpack_rle_row();
		// uint32_t* row_start = (uint32_t*)&image_data_table[sprite_id].frames[frame_idx][0];
#endif
		uint32_t* from;
		for(int y_w = y_start; y_w < y_end; y_w += CAT_TILE_SIZE)
		{
			uint32_t* to = (uint32_t*)&FRAMEBUFFER[(y_w + dy) * LCD_SCREEN_W];
			UNROLL1LI;
		}
	}
}

void CAT_spriter_cleanup()
{
#ifdef CAT_DESKTOP
	CAT_free(spriter.framebuffer);
#endif
}


/////////////////////awhhhhhhehhhhh

#ifdef CAT_EMBEDDED
#include "menu_graph_rendering.c"
#endif


//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

CAT_anim_table anim_table;

void CAT_anim_table_init()
{
	for(int i = 0; i < CAT_ATLAS_MAX_LENGTH; i++)
	{
		anim_table.frame_idx[i] = 0;
		anim_table.loop[i] = true;
		anim_table.reverse[i] = false;
		anim_table.dirty[i] = true;
	}

	anim_table.timer = 0;
}

void CAT_anim_toggle_loop(int sprite_id, bool toggle)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid animation: %d\n", sprite_id);
		return;
	}

	anim_table.loop[sprite_id] = toggle;
}

void CAT_anim_toggle_reverse(int sprite_id, bool toggle)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid animation: %d\n", sprite_id);
		return;
	}

	anim_table.reverse[sprite_id] = toggle;
}

bool CAT_anim_finished(int sprite_id)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		return true;
	}

	CAT_sprite* sprite = &atlas.data[sprite_id];
	return anim_table.frame_idx[sprite_id] == sprite->frame_count-1;
}

void CAT_anim_reset(int sprite_id)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		return;
	}

	anim_table.frame_idx[sprite_id] = 0;
}

CAT_draw_queue draw_queue =
{
	.length = 0
};

void CAT_draw_queue_insert(int idx, int sprite_id, int frame_idx, int layer, int x, int y, int mode)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid sprite: %d\n", sprite_id);
		return;
	}
	if(draw_queue.length >= CAT_DRAW_QUEUE_MAX_LENGTH)
	{
		CAT_printf("[WARNING] Attempted add to full draw queue\n");
		return;
	}

	for(int i = draw_queue.length; i > idx; i--)
	{
		draw_queue.jobs[i] = draw_queue.jobs[i-1];
	}
	draw_queue.jobs[idx] = (CAT_draw_job) {sprite_id, frame_idx, layer, x, y, mode};
	draw_queue.length += 1;
}

int CAT_draw_queue_add(int sprite_id, int frame_idx, int layer, int x, int y, int mode)
{
	int insert_idx = draw_queue.length;
	for(int i = 0; i < insert_idx; i++)
	{
		CAT_draw_job other = draw_queue.jobs[i];
		if(layer < other.layer || (layer == other.layer && y < other.y))
		{
			insert_idx = i;
			break;
		}
	}

	CAT_draw_queue_insert(insert_idx, sprite_id, frame_idx, layer, x, y, mode);
	return insert_idx;
}

void CAT_draw_queue_submit(int cycle)
{
	if (cycle==0)
	{
		anim_table.timer += CAT_get_delta_time();
		if(anim_table.timer >= 0.15f)
		{
			for(int i = 0; i < draw_queue.length; i++)
			{
				CAT_draw_job* job = &draw_queue.jobs[i];
				if(job->frame_idx != -1)
					continue;
				int sprite_id = job->sprite_id;
				if(!anim_table.dirty[sprite_id])
					continue;
			
				CAT_sprite* sprite = CAT_sprite_get(sprite_id);
				if(anim_table.frame_idx[sprite_id] < sprite->frame_count-1)
					anim_table.frame_idx[sprite_id] += 1;
				else if(anim_table.loop[sprite_id])
					anim_table.frame_idx[sprite_id] = 0;
				
				anim_table.dirty[sprite_id] = false;
			}
			anim_table.timer = 0.0f;
		}

		for(int i = 0; i < draw_queue.length; i++)
		{
			CAT_draw_job* job = &draw_queue.jobs[i];
			int sprite_id = job->sprite_id;
			anim_table.dirty[sprite_id] = true;
		}
	}

	for(int i = 0; i < draw_queue.length; i++)
	{
		CAT_draw_job* job = &draw_queue.jobs[i];
		int sprite_id = job->sprite_id;
		CAT_sprite* sprite = &atlas.data[sprite_id];
		if(job->frame_idx == -1)
		{
			job->frame_idx = anim_table.frame_idx[sprite_id];
			if(anim_table.reverse[sprite_id])
				job->frame_idx = sprite->frame_count-1-job->frame_idx;
		}
		spriter.mode = job->mode;
		CAT_draw_sprite(sprite_id, job->frame_idx, job->x, job->y);
	}
}


//////////////////////////////////////////////////////////////////////////
// ANIMATION MACHINE

void CAT_animachine_init(CAT_animachine_state* state, int enai, int tiai, int exai)
{
	state->signal = ENTER;
	state->enter_anim_id = enai;
	state->tick_anim_id = tiai;
	state->exit_anim_id = exai;
	state->next = NULL;
}

void CAT_animachine_transition(CAT_animachine_state** spp, CAT_animachine_state* next)
{
	if(next == NULL)
	{
		*spp = NULL;
		return;
	}

	CAT_animachine_state* sp = *spp;
	if(sp != NULL)
	{
		if(sp->signal != DONE)
			sp->signal = EXIT;
		sp->next = next;
	}
	else
	{
		next->signal = ENTER;
		CAT_anim_reset(next->enter_anim_id);
		CAT_anim_reset(next->tick_anim_id);
		CAT_anim_reset(next->exit_anim_id);
		*spp = next;
	}
}

int CAT_animachine_tick(CAT_animachine_state** spp)
{
	if(*spp == NULL)
		return -1;
	CAT_animachine_state* sp = *spp;

	switch(sp->signal)
	{
		case ENTER:
		{
			if(!CAT_anim_finished(sp->enter_anim_id))
				return sp->enter_anim_id;
			sp->signal = TICK;
			break;
		}
		case TICK:
		{
			if(sp->tick_anim_id != -1)
				return sp->tick_anim_id;
			sp->signal = EXIT;
			break;
		}
		case EXIT:
		{
			if(!CAT_anim_finished(sp->exit_anim_id))
				return sp->exit_anim_id;
			sp->signal = DONE;
			break;
		}
		default:
		{
			if(sp->next != NULL)
			{
				CAT_animachine_state* next = sp->next;
				sp->next = NULL;

				next->signal = ENTER;
				CAT_anim_reset(next->enter_anim_id);
				CAT_anim_reset(next->tick_anim_id);
				CAT_anim_reset(next->exit_anim_id);
				*spp = next;
			}
			break;
		}
	}

	return sp->exit_anim_id != -1 ? sp->exit_anim_id : sp->tick_anim_id;
}

void CAT_animachine_kill(CAT_animachine_state** spp)
{
	if(*spp == NULL)
		return;

	if((*spp)->signal != DONE)
		(*spp)->signal = EXIT;
}

bool CAT_animachine_is_in(CAT_animachine_state** spp, CAT_animachine_state* state)
{
	if(*spp == NULL)
		return false;

	return (*spp) == state;
}

bool CAT_animachine_is_ticking(CAT_animachine_state** spp)
{
	if(*spp == NULL)
		return false;

	return (*spp)->signal == TICK;
}


//////////////////////////////////////////////////////////////////////////
// DECLARATIONS AND DEFINITIONS

// MACHINES
CAT_animachine_state* pet_asm;

CAT_animachine_state AS_idle;
CAT_animachine_state AS_walk;
CAT_animachine_state AS_crit;

CAT_animachine_state AS_adjust_in;
CAT_animachine_state AS_approach;
CAT_animachine_state AS_adjust_out;

CAT_animachine_state AS_eat;
CAT_animachine_state AS_study;
CAT_animachine_state AS_play;

CAT_animachine_state AS_vig_up;
CAT_animachine_state AS_foc_up;
CAT_animachine_state AS_spi_up;

CAT_animachine_state* react_asm;
CAT_animachine_state AS_react;

void CAT_sprite_mass_define()
{
	// PET STATES
	CAT_anim_toggle_loop(pet_high_vig_in_sprite, false);
	CAT_anim_toggle_reverse(pet_high_vig_out_sprite, true);
	CAT_anim_toggle_loop(pet_high_vig_out_sprite, false);

	CAT_anim_toggle_loop(pet_crit_vig_in_sprite, false);
	CAT_anim_toggle_reverse(pet_crit_vig_out_sprite, true);
	CAT_anim_toggle_loop(pet_crit_vig_out_sprite, false);

	CAT_anim_toggle_loop(pet_crit_foc_in_sprite, false);
	CAT_anim_toggle_reverse(pet_crit_foc_out_sprite, true);
	CAT_anim_toggle_loop(pet_crit_foc_out_sprite, false);

	CAT_anim_toggle_loop(pet_crit_spi_in_sprite, false);
	CAT_anim_toggle_reverse(pet_crit_spi_out_sprite, true);
	CAT_anim_toggle_loop(pet_crit_spi_out_sprite, false);

	// PET ACTIONS
	CAT_anim_toggle_loop(pet_eat_in_sprite, false);
	CAT_anim_toggle_reverse(pet_eat_out_sprite, true);
	CAT_anim_toggle_loop(pet_eat_out_sprite, false);

	CAT_anim_toggle_loop(pet_study_in_sprite, false);
	CAT_anim_toggle_reverse(pet_study_out_sprite, true);
	CAT_anim_toggle_loop(pet_study_out_sprite, false);

	CAT_printf("[INFO] %d sprites initialized\n", atlas.length);


	// MACHINES
	CAT_animachine_init(&AS_idle, -1, pet_idle_sprite, -1);
	CAT_animachine_init(&AS_walk, -1, pet_walk_sprite, -1);
	CAT_animachine_init(&AS_crit, pet_crit_vig_in_sprite, pet_crit_vig_sprite, pet_crit_vig_out_sprite);

	CAT_animachine_init(&AS_adjust_in, -1, -1, pet_idle_sprite);
	CAT_animachine_init(&AS_approach, -1, pet_walk_sprite, -1);
	CAT_animachine_init(&AS_adjust_out, -1, -1, pet_idle_sprite);

	CAT_animachine_init(&AS_eat, pet_eat_in_sprite, pet_eat_sprite, pet_eat_out_sprite);
	CAT_animachine_init(&AS_study, pet_study_in_sprite, pet_study_sprite, pet_study_out_sprite);
	CAT_animachine_init(&AS_play, -1, pet_play_a_sprite, -1);

	CAT_animachine_init(&AS_vig_up, -1, -1, pet_vig_up_sprite);
	CAT_animachine_init(&AS_foc_up, -1, -1, pet_foc_up_sprite);
	CAT_animachine_init(&AS_spi_up, -1, -1, pet_spi_up_sprite);

	CAT_animachine_init(&AS_react, -1, mood_good_sprite, -1);
}
