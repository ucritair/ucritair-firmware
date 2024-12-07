#include "cat_sprite.h"

#include <stdint.h>
#include <string.h>
#include "cat_core.h"
#include <stdint.h>
#include "cat_math.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
// ATLAS AND SPRITER

#ifndef CAT_BAKED_ASSETS
CAT_atlas atlas;
#else
#ifdef CAT_DESKTOP
#include "../../script/images.c"
#include <stdio.h>
#else
extern const CAT_baked_sprite image_data_table[];
extern uint16_t rle_work_region[];
#include "lcd_driver.h"
#endif
#endif

#ifndef CAT_EMBEDDED
#define LCD_FRAMEBUFFER_H LCD_SCREEN_H
#endif

void CAT_atlas_init()
{
#ifndef CAT_BAKED_ASSETS
	atlas.length = 0;
#endif
}

#ifndef CAT_BAKED_ASSETS

#include "png.h"

int CAT_sprite_init(const char* path, int frame_count)
{
	if(atlas.length >= CAT_ATLAS_MAX_LENGTH)
	{
		CAT_printf("[WARNING] Attempted add to full atlas\n");
		return -1;
	}

	FILE* file = fopen(path, "rb");
	if(file == NULL)
	{
		printf("%s not found! Loading null sprite\n", path);
		file = fopen("sprites/none_24x24.png", "rb");
	}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);
	png_init_io(png, file);
	png_set_sig_bytes(png, 0);
	int png_transforms =
		PNG_TRANSFORM_STRIP_16 |
		PNG_TRANSFORM_PACKING |
		PNG_TRANSFORM_EXPAND;
	png_read_png(png, info, png_transforms, NULL);

	png_uint_32 width;
	png_uint_32 height;
	png_get_IHDR(png, info, &width, &height, NULL, NULL, NULL, NULL, NULL);

	png_bytepp rows = png_get_rows(png, info);
	uint16_t* pixels = CAT_malloc(sizeof(uint16_t) * width * height);
	for(int y = 0; y < height; y++)
	{
		uint8_t* r_row = rows[y];
		uint16_t* w_row = &pixels[y * width];
		for(int x = 0; x < width; x++)
		{
			uint8_t r = r_row[x*4+0];
			uint8_t g = r_row[x*4+1];
			uint8_t b = r_row[x*4+2];
			uint8_t a = r_row[x*4+3];
			uint16_t rgb_565 = RGB8882565(r, g, b);
			w_row[x] = a >= 255 ? rgb_565 : 0xdead;
		}
	}
	png_destroy_read_struct(&png, &info, NULL);
	
	CAT_sprite sprite;
	sprite.pixels = pixels;
	sprite.duplicate = false;
	sprite.width = width;
	sprite.height = height / frame_count;
	sprite.frame_count = frame_count;
	sprite.frame_idx = 0;
	sprite.loop = true;
	sprite.reverse = false;
	sprite.needs_update = true;

	int sprite_id = atlas.length;
	atlas.table[sprite_id] = sprite;
	atlas.length += 1;
	return sprite_id;
}

int CAT_sprite_copy(int sprite_id, bool loop, bool reverse)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid sprite_id: %d\n", sprite_id);
		return -1;
	}

	CAT_sprite copy = atlas.table[sprite_id];
	copy.duplicate = true;
	copy.frame_idx = reverse ? copy.frame_count-1 : 0;
	copy.loop = loop;
	copy.reverse = reverse;
	copy.needs_update = true;
	
	int copy_id = atlas.length;
	atlas.table[copy_id] = copy;
	atlas.length += 1;
	return copy_id;
}

void CAT_atlas_cleanup()
{
	for(int i = 0; i < atlas.length; i++)
	{
		CAT_sprite* sprite = &atlas.table[i];
		if(!sprite->duplicate)
			CAT_free(sprite->pixels);
	}
}

#endif

CAT_sprite* CAT_sprite_get(int sprite_id)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid sprite_id: %d\n", sprite_id);
		return NULL;
	}

	return &atlas.table[sprite_id];
}

CAT_spriter spriter;

void CAT_spriter_init()
{
#ifdef CAT_DESKTOP
	spriter.framebuffer = CAT_malloc(sizeof(uint16_t) * LCD_SCREEN_W * LCD_SCREEN_H);
#define FRAMEBUFFER spriter.framebuffer
#else
#define FRAMEBUFFER lcd_framebuffer
#endif
	spriter.mode = CAT_DRAW_MODE_DEFAULT;
}

#ifdef CAT_BAKED_ASSETS
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
		CAT_printf("[ERROR] reference to invalid sprite_id: %d\n", sprite_id);
		return;
	}

	CAT_sprite sprite = atlas.table[sprite_id];
	int w = sprite.width;
	int h = sprite.height;

#ifdef CAT_BAKED_ASSETS
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

	if (y_end >= LCD_FRAMEBUFFER_H) y_end = LCD_FRAMEBUFFER_H;
	
	int dy = 0; // only on PC
	int inc_dir = (spriter.mode & CAT_DRAW_MODE_REFLECT_X)?-1:1;
	int initial_offset = inc_dir==1?0:w-1;

	while (y < y_end)
	{
#ifdef CAT_BAKED_ASSETS
		unpack_rle_row();
#endif
		
		if(y < 0)
		{
			y++;
			dy++;
			continue;
		}

#ifdef CAT_BAKED_ASSETS
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

			if(x_w < 0)
			{
				
			}
			else
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
		CAT_printf("[ERROR] reference to invalid sprite_id: %d\n", sprite_id);
		return;
	}

	CAT_sprite sprite = atlas.table[sprite_id];

#ifndef CAT_BAKED_ASSETS
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

#ifdef CAT_BAKED_ASSETS
	init_rle_decode(&image_data_table[sprite_id], frame_idx, sprite.width);
#endif

#if CAT_TILE_SIZE != 16
#error adjust tiler (rep count)
#endif

#ifdef CAT_BAKED_ASSETS
#define RESETPTR from = (uint32_t*)rle_work_region;
#else
#define RESETPTR from = (uint32_t*)&frame[dy * CAT_TILE_SIZE];
#endif

#if (LCD_SCREEN_W/CAT_TILE_SIZE) != 15
#error adjust tiler (rep count)
#endif

#ifdef CAT_EMBEDDED
#if ((LCD_SCREEN_H/LCD_FRAMEBUFFER_SEGMENTS)%16) != 0
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
#ifdef CAT_BAKED_ASSETS
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

CAT_draw_queue draw_queue =
{
	.length = 0
};

float anim_period = 0.15f;
float anim_timer = 0.0f;

void CAT_anim_toggle_loop(int sprite_id, bool toggle)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid sprite_id: %d\n", sprite_id);
		return;
	}

	CAT_sprite* sprite = &atlas.table[sprite_id];
	sprite->loop = toggle;
}

void CAT_anim_toggle_reverse(int sprite_id, bool toggle)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid sprite_id: %d\n", sprite_id);
		return;
	}

	CAT_sprite* sprite = &atlas.table[sprite_id];
	sprite->reverse = toggle;
	if(sprite->reverse)
		sprite->frame_idx = sprite->frame_count-1;
	else
		sprite->frame_idx = 0;
}

bool CAT_anim_finished(int sprite_id)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		return true;
	}

	CAT_sprite* sprite = &atlas.table[sprite_id];
	if(sprite->reverse)
		return sprite->frame_idx == 0;
	else
		return sprite->frame_idx == sprite->frame_count-1;
}

void CAT_anim_reset(int sprite_id)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		return;
	}

	CAT_sprite* sprite = &atlas.table[sprite_id];
	if(sprite->reverse)
		sprite->frame_idx = sprite->frame_count-1;
	else
		sprite->frame_idx = 0;
}

void CAT_draw_queue_insert(int idx, int sprite_id, int frame_idx, int layer, int x, int y, int mode)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
	{
		CAT_printf("[ERROR] reference to invalid sprite_id: %d\n", sprite_id);
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

int CAT_draw_queue_animate(int sprite_id, int layer, int x, int y, int mode)
{
	return CAT_draw_queue_add(sprite_id, -1, layer, x, y, mode);
}

void CAT_draw_queue_submit(int cycle)
{
	if (cycle==0)
	{
		anim_timer += CAT_get_delta_time();
		if(anim_timer >= anim_period)
		{
			for(int i = 0; i < draw_queue.length; i++)
			{
				CAT_draw_job* job = &draw_queue.jobs[i];
				if(job->frame_idx != -1)
					continue;

				CAT_sprite* sprite = &atlas.table[job->sprite_id];
				if(!sprite->needs_update)
					continue;
				
				int frame_start = sprite->reverse ? sprite->frame_count-1 : 0;
				int frame_end = sprite->reverse ? 0 : sprite->frame_count-1;
				int frame_dir = sprite->reverse ? -1 : 1;
				if(sprite->frame_idx != frame_end)
					sprite->frame_idx += frame_dir;
				else if(sprite->loop)
					sprite->frame_idx = frame_start;

				sprite->needs_update = false;
			}
			anim_timer = 0.0f;
		}

		for (int i = 0; i < draw_queue.length; i++)
		{
			atlas.table[draw_queue.jobs[i].sprite_id].needs_update = true;
		}
	}

	for(int i = 0; i < draw_queue.length; i++)
	{
		CAT_draw_job* job = &draw_queue.jobs[i];
		CAT_sprite* sprite = &atlas.table[job->sprite_id];
		if(job->frame_idx == -1)
			job->frame_idx = sprite->frame_idx;
			
		spriter.mode = job->mode;
		CAT_draw_sprite(job->sprite_id, job->frame_idx, job->x, job->y);
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

void CAT_roundberry(int xi, int yi, int r, uint16_t c)
{
	int xc = xi + r;
	int yc = yi + r;
	int xf = xc + r;
	int yf = yc + r;
	for(int y = yi; y < yf; y++)
	{
		for(int x = xi; x < xf; x++)
		{
			int dx = x - xc;
			int dy = y - yc;
			if((dx * dx + dy * dy) < r*r)
			{
				int idx = y * LCD_SCREEN_W + x;
				FRAMEBUFFER[idx] = c;
			}
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

#ifdef CAT_DESKTOP
float depthbuffer[LCD_SCREEN_W * LCD_SCREEN_H];

void CAT_depthberry()
{
	for(int i = 0; i < LCD_SCREEN_W * LCD_SCREEN_H; i++)
		depthbuffer[i] = 1.0f;
}

bool depth_test_write(int x, int y, float z)
{
	int idx = y * LCD_SCREEN_W + x;
	float z0 = depthbuffer[idx];
	if(z <= z0)
	{
		depthbuffer[idx] = z;
		return true;
	}
	return false;
}

int cross2d(int x0, int y0, int x1, int y1)
{
	return x0 * y1 - y0 * x1;
}

void CAT_triberry
(
	int xa, int ya, float za,
	int xb, int yb, float zb,
	int xc, int yc, float zc,
	uint16_t c
)
{
	int min_x = min(min(xa, xb), xc);
	int min_y = min(min(ya, yb), yc);
	int max_x = max(max(xa, xb), xc);
	int max_y = max(max(ya, yb), yc);

	if(max_x < 0 || max_y < 0)
		return;
	if(min_x >= LCD_SCREEN_W || min_y >= LCD_SCREEN_H)
		return;

	int xab = xb - xa;
	int yab = yb - ya;
	int xac = xc - xa;
	int yac = yc - ya;
	float abXac = (float) cross2d(xab, yab, xac, yac);
	if(abXac == 0)
		return;
	
	for(int y = min_y; y <= max_y; y++)
	{
		if(y < 0 || y >= LCD_SCREEN_H)
			continue;
		int yp = y - ya;
	
		for(int x = min_x; x <= max_x; x++)
		{
			if(x < 0 || x >= LCD_SCREEN_W)
				continue;
			int xp = x - xa;
		
			float v = (float) cross2d(xp, yp, xac, yac) / abXac;
			if(v < 0 || v > 1)
				continue;
			float w = (float) cross2d(xab, yab, xp, yp) / abXac;
			if(w < 0 || w > 1)
				continue;
			float u = 1.0f - v - w;
			if(u < 0 || u > 1)
				continue;
				
			float z = u * za + v * zb + w * zc;
			if(depth_test_write(x, y, z))
			{
				FRAMEBUFFER[y * LCD_SCREEN_W + x] = c;
			}
		}
	}
}
#else
void CAT_depthberry()
{
	return;
}

void CAT_triberry
(
	int xa, int ya, float za,
	int xb, int yb, float zb,
	int xc, int yc, float zc,
	uint16_t c
)
{
	CAT_lineberry(xa, ya, xb, yb, c);
	CAT_lineberry(xb, yb, xc, yc, c);
	CAT_lineberry(xc, yc, xa, ya, c);
}
#endif

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
// DECLARATIONS

// TILESETS
int base_wall_sprite;
int sky_wall_sprite;
int base_floor_sprite;
int grass_floor_sprite;
int ash_floor_sprite;

int glyph_sprite;
int strike_sprite;
int tab_sprite;

// ICONS
int icon_pointer_sprite;
int icon_a_sprite;
int icon_b_sprite;
int icon_n_sprite;
int icon_e_sprite;
int icon_s_sprite;
int icon_w_sprite;
int icon_start_sprite;
int icon_select_sprite;
int icon_enter_sprite;
int icon_exit_sprite;
int icon_plot_sprite;
int icon_equip_sprite;
int icon_input_sprite;

int icon_item_key_sprite;
int icon_item_food_sprite;
int icon_item_book_sprite;
int icon_item_toy_sprite;
int icon_item_prop_sprite;
int icon_item_gear_sprite;
int icon_coin_sprite;

int icon_vig_sprite;
int icon_foc_sprite;
int icon_spi_sprite;
int pip_vig_sprite;
int pip_foc_sprite;
int pip_spi_sprite;
int pip_empty_sprite;

int icon_temp_sprite;
int icon_co2_sprite;
int icon_pm_sprite;
int icon_voc_sprite;
int icon_nox_sprite;

int icon_mask_sprite;
int icon_pure_sprite;
int icon_uv_sprite;

int icon_nosmoke_sprite;
int icon_ee_sprite;
int icon_aq_ccode_sprite;
int icon_cell_sprite;

int icon_feed_sprite;
int icon_study_sprite;
int icon_play_sprite;
int icon_deco_sprite;
int icon_menu_sprite;

// CURSORS
int cursor_sprite;
int tile_hl_sprite;

int cursor_add_sprite;
int tile_hl_add_sprite;

int cursor_flip_sprite;
int tile_hl_flip_sprite;
int tile_mark_flip_sprite;

int cursor_remove_sprite;
int tile_hl_rm_sprite;
int tile_mark_rm_sprite;

int button_hl_sprite;
int touch_hl_sprite;

// TOOLS
int bread_sprite;
int milk_sprite;
int soup_sprite;
int green_curry_sprite;
int red_curry_sprite;
int padkaprow_sprite;
int sausage_sprite;
int coffee_sprite;
int salad_sprite;
int pill_vig_sprite;
int pill_foc_sprite;
int pill_spi_sprite;
int cigarette_sprite;

int book_static_sprite;
int book_study_sprite;

int toy_duck_sprite;
int toy_baseball_sprite;
int toy_basketball_sprite;
int toy_golf_sprite;
int toy_puzzle_sprite;

int coin_static_sprite;
int coin_world_sprite;

// FIXED PROPS
int window_dawn_sprite;
int window_day_sprite;
int window_day_high_aq_sprite;
int window_day_low_aq_sprite;
int window_night_sprite;
int vending_sprite;
int arcade_sprite;

// GAMEPLAY PROPS
int gpu_sprite;
int uv_sprite;
int purifier_sprite;

// DECO PROPS
int coffeemaker_sprite;
int fan_a_sprite;
int fan_b_sprite;
int lantern_sprite;
int laptop_sprite;
int chess_sprite;

int table_lg_sprite;
int table_sm_sprite;
int chair_wood_sprite;
int chair_stone_sprite;
int stool_wood_sprite;
int stool_stone_sprite;
int stool_gold_sprite;

int bowl_stone_sprite;
int bowl_gold_sprite;
int vase_stone_sprite;
int vase_gold_sprite;

int succulent_sprite;
int bush_plain_sprite;
int bush_daisy_sprite;
int bush_lilac_sprite;
int plant_green_sprite;
int plant_maroon_sprite;
int plant_purple_sprite;
int plant_yellow_sprite;
int flower_vig_sprite;
int flower_foc_sprite;
int flower_spi_sprite;

int crystal_blue_lg_sprite;
int crystal_green_lg_sprite;
int crystal_purple_lg_sprite;

int effigy_blue_sprite;
int effigy_purple_sprite;
int effigy_sea_sprite;

int poster_zk_sprite;
int pixel_sprite;
int padkaprop_sprite;

// PET STATES
int pet_idle_sprite;
int pet_walk_sprite;

int pet_idle_high_vig_sprite;
int pet_walk_high_vig_sprite;
int pet_idle_high_foc_sprite;
int pet_walk_high_foc_sprite;
int pet_idle_high_spi_sprite;
int pet_walk_high_spi_sprite;
int pet_wings_out_sprite;
int pet_wings_in_sprite;

int pet_idle_low_vig_sprite;
int pet_walk_low_vig_sprite;
int pet_idle_low_foc_sprite;
int pet_walk_low_foc_sprite;
int pet_idle_low_spi_sprite;
int pet_walk_low_spi_sprite;

int pet_crit_vig_in_sprite;
int pet_crit_vig_sprite;
int pet_crit_vig_out_sprite;

int pet_crit_foc_in_sprite;
int pet_crit_foc_sprite;
int pet_crit_foc_out_sprite;

int pet_crit_spi_in_sprite;
int pet_crit_spi_sprite;
int pet_crit_spi_out_sprite;

int pet_vig_up_sprite;
int pet_foc_up_sprite;
int pet_spi_up_sprite;

// PET ACTIONS
int pet_eat_in_sprite;
int pet_eat_sprite;
int pet_eat_out_sprite;

int pet_study_in_sprite;
int pet_study_sprite;
int pet_study_out_sprite;

int pet_play_a_sprite;
int pet_play_b_sprite;
int pet_play_c_sprite;

// PET MOODS
int mood_low_vig_sprite;
int mood_low_foc_sprite;
int mood_low_spi_sprite;
int mood_good_sprite;
int mood_bad_sprite;

// SNAKE
int snake_head_sprite;
int snake_body_sprite;
int snake_corner_sprite;
int snake_tail_sprite;


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

#ifndef CAT_BAKED_ASSETS
#ifdef CAT_REBUILD_ATLAS
#define INIT_SPRITE(name, path, frames) name = CAT_sprite_init(path, frames);\
										printf("BAKE-INIT: (%d, \"%s\", \"%s\", %d, %d, %d)\n", name, #name, path, frames, atlas.table[name].width, atlas.table[name].height);
#define COPY_SPRITE(name, from, loop, reverse) name = CAT_sprite_copy(from, loop, reverse);\
										printf("BAKE-COPY: (%d, \"%s\", %d, %d, %d)\n", name, #name, from, loop, reverse);
#else
#define INIT_SPRITE(name, path, frames) name = CAT_sprite_init(path, frames);
#define COPY_SPRITE(name, from, loop, reverse) name = CAT_sprite_copy(from, loop, reverse);
#endif
#else
int sprite_count = 0;
#define INIT_SPRITE(name, path, frames) name = sprite_count++;
#define COPY_SPRITE(name, from, loop, reverse) name = sprite_count++;
#endif

void CAT_sprite_mass_define()
{
#ifdef CAT_DESKTOP
	setvbuf(stdout, NULL, _IONBF, 0);
#endif

	// TILESETS
	INIT_SPRITE(base_wall_sprite, "sprites/wall_basic.png", 3);
	INIT_SPRITE(sky_wall_sprite, "sprites/wall_sky.png", 8);
	INIT_SPRITE(base_floor_sprite, "sprites/tile_basic.png", 3);
	INIT_SPRITE(grass_floor_sprite, "sprites/tile_grass.png", 21);
	INIT_SPRITE(ash_floor_sprite, "sprites/tile_ash.png", 6);

	INIT_SPRITE(glyph_sprite, "sprites/glyphs.png", 96);
	INIT_SPRITE(strike_sprite, "sprites/strikethrough.png", 1);
	INIT_SPRITE(tab_sprite, "sprites/gui_tab.png", 2);

	// ICONS
	INIT_SPRITE(icon_pointer_sprite, "sprites/icon_pointer.png", 1);
	INIT_SPRITE(icon_a_sprite, "sprites/A Button_Both.png", 2);
	INIT_SPRITE(icon_b_sprite, "sprites/B Button_Both.png", 2);
	INIT_SPRITE(icon_n_sprite, "sprites/Up_Arrow_Both.png", 2);
	INIT_SPRITE(icon_e_sprite, "sprites/Right Arrow_Both.png", 2);
	INIT_SPRITE(icon_s_sprite, "sprites/Down Arrow_Both.png", 2);
	INIT_SPRITE(icon_w_sprite, "sprites/Left Arrow_Both.png", 2);
	INIT_SPRITE(icon_start_sprite, "sprites/Start Button_Both.png", 2);
	INIT_SPRITE(icon_select_sprite, "sprites/Select Button_Both.png", 2);
	INIT_SPRITE(icon_enter_sprite, "sprites/icon_enter.png", 1);
	INIT_SPRITE(icon_exit_sprite, "sprites/icon_exit.png", 1);
	INIT_SPRITE(icon_plot_sprite, "sprites/icon_plot.png", 1);
	INIT_SPRITE(icon_equip_sprite, "sprites/icon_equip.png", 2);
	INIT_SPRITE(icon_input_sprite, "sprites/icon_inputs.png", 8);

	INIT_SPRITE(icon_item_key_sprite, "sprites/icon_item_key.png", 1);
	INIT_SPRITE(icon_item_food_sprite, "sprites/icon_item_food.png", 1);
	INIT_SPRITE(icon_item_book_sprite, "sprites/icon_item_book.png", 1);
	INIT_SPRITE(icon_item_toy_sprite, "sprites/icon_item_toy.png", 1);
	INIT_SPRITE(icon_item_prop_sprite, "sprites/icon_item_prop.png", 1);
	INIT_SPRITE(icon_item_gear_sprite, "sprites/icon_item_gear.png", 1);
	INIT_SPRITE(icon_coin_sprite, "sprites/icon_coin.png", 1);

	INIT_SPRITE(icon_vig_sprite, "sprites/STAT_VIGOR2424.png", 1);
	INIT_SPRITE(icon_foc_sprite, "sprites/STAT_FOCUS2424.png", 1);
	INIT_SPRITE(icon_spi_sprite, "sprites/STAT_SPIRIT2424.png", 1);
	INIT_SPRITE(pip_vig_sprite, "sprites/cell_vig.png", 1);
	INIT_SPRITE(pip_foc_sprite, "sprites/cell_foc.png", 1);
	INIT_SPRITE(pip_spi_sprite, "sprites/cell_spi.png", 1);
	INIT_SPRITE(pip_empty_sprite, "sprites/cell_empty.png", 1);

	INIT_SPRITE(icon_temp_sprite, "sprites/icon_temp.png", 3);
	INIT_SPRITE(icon_co2_sprite, "sprites/icon_co2.png", 3);
	INIT_SPRITE(icon_pm_sprite, "sprites/icon_pm.png", 3);
	INIT_SPRITE(icon_voc_sprite, "sprites/icon_voc.png", 3);
	INIT_SPRITE(icon_nox_sprite, "sprites/icon_nox.png", 3);

	INIT_SPRITE(icon_mask_sprite, "sprites/aq-protection-mask.png", 1);
	INIT_SPRITE(icon_pure_sprite, "sprites/aq-protection-purifier.png", 1);
	INIT_SPRITE(icon_uv_sprite, "sprites/aq-protection-uv.png", 1);

	INIT_SPRITE(icon_nosmoke_sprite, "sprites/nosmoke.png", 1);
	INIT_SPRITE(icon_ee_sprite, "sprites/ee_logo.png", 1);
	INIT_SPRITE(icon_aq_ccode_sprite, "sprites/icon_aq_ccode.png", 3);
	INIT_SPRITE(icon_cell_sprite, "sprites/icon_cell_sprite.png", 2);

	INIT_SPRITE(icon_feed_sprite, "sprites/Stat_Refill_Vigor_Button.png", 2);
	INIT_SPRITE(icon_study_sprite, "sprites/Stat_Refill_Focus_Button.png", 2);
	INIT_SPRITE(icon_play_sprite, "sprites/Stat_Refill_Spirit_Button.png", 2);
	INIT_SPRITE(icon_deco_sprite, "sprites/sbut_deco.png", 2);
	INIT_SPRITE(icon_menu_sprite, "sprites/sbut_menu.png", 2);

	// CURSORS
	INIT_SPRITE(cursor_sprite, "sprites/cursor_room_ornate.png", 1);
	INIT_SPRITE(tile_hl_sprite, "sprites/tile_hl.png", 1);

	INIT_SPRITE(cursor_add_sprite, "sprites/cursor_add.png", 1);
	INIT_SPRITE(tile_hl_add_sprite, "sprites/tile_hl_add.png", 1);

	INIT_SPRITE(cursor_flip_sprite, "sprites/cursor_flip.png", 1);
	INIT_SPRITE(tile_hl_flip_sprite, "sprites/tile_hl_flip.png", 1);
	INIT_SPRITE(tile_mark_flip_sprite, "sprites/tile_mark_flip.png", 1);

	INIT_SPRITE(cursor_remove_sprite, "sprites/cursor_remove.png", 1);
	INIT_SPRITE(tile_hl_rm_sprite, "sprites/tile_hl_rm.png", 1);
	INIT_SPRITE(tile_mark_rm_sprite, "sprites/tile_mark_rm.png", 1);
	
	INIT_SPRITE(button_hl_sprite, "sprites/sbut_hl_sprite.png", 1);
	INIT_SPRITE(touch_hl_sprite, "sprites/touch_ring_sprite.png", 1);
	
	// TOOLS
	INIT_SPRITE(bread_sprite, "sprites/food_bread.png", 1);
	INIT_SPRITE(coffee_sprite, "sprites/food_coffee_sm.png", 1);
	INIT_SPRITE(milk_sprite, "sprites/food_milk.png", 1);
	INIT_SPRITE(soup_sprite, "sprites/food_soup.png", 1);
	INIT_SPRITE(salad_sprite, "sprites/food_salad_sm.png", 1);
	INIT_SPRITE(sausage_sprite, "sprites/food_sausage_sm.png", 1);
	INIT_SPRITE(green_curry_sprite, "sprites/food_curry_green.png", 1);
	INIT_SPRITE(red_curry_sprite, "sprites/food_curry_red.png", 1);
	INIT_SPRITE(padkaprow_sprite, "sprites/food_padkrakow_sm.png", 1);
	INIT_SPRITE(pill_vig_sprite, "sprites/seed_vigor_sm.png", 1);
	INIT_SPRITE(pill_foc_sprite, "sprites/seed_focus_sm.png", 1);
	INIT_SPRITE(pill_spi_sprite, "sprites/seed_spirit_sm.png", 1);
	INIT_SPRITE(cigarette_sprite, "sprites/prop_cigarette.png", 1);

	INIT_SPRITE(book_static_sprite, "sprites/read_book_upright.png", 1);
	INIT_SPRITE(book_study_sprite, "sprites/read_book_turn_a.png", 8);

	INIT_SPRITE(toy_duck_sprite, "sprites/toy_sm_duck.png", 1);
	INIT_SPRITE(toy_baseball_sprite, "sprites/toy_baseball.png", 1);
	INIT_SPRITE(toy_basketball_sprite, "sprites/toy_sm_basketball.png", 1);
	INIT_SPRITE(toy_golf_sprite, "sprites/toy_sm_golf.png", 1);
	INIT_SPRITE(toy_puzzle_sprite, "sprites/toy_sm_puzzle.png", 1);

	INIT_SPRITE(coin_static_sprite, "sprites/coin.png", 1);
	INIT_SPRITE(coin_world_sprite, "sprites/coin_a.png", 4);

	// FIXED PROPS
	INIT_SPRITE(window_dawn_sprite, "sprites/prop_wall_window_dawn.png", 1);
	INIT_SPRITE(window_day_sprite, "sprites/prop_wall_window_day.png", 1);
	INIT_SPRITE(window_day_high_aq_sprite, "sprites/prop_wall_window_aqi_high_a.png", 8);
	INIT_SPRITE(window_day_low_aq_sprite, "sprites/prop_wall_window_aqi_mid.png", 1);
	INIT_SPRITE(window_night_sprite, "sprites/prop_wall_window_dark.png", 1);
	INIT_SPRITE(vending_sprite, "sprites/interact_vending_items_a.png", 12);
	INIT_SPRITE(arcade_sprite, "sprites/arcade_a.png", 2);

	// GAMEPLAY PROPS
	INIT_SPRITE(gpu_sprite, "sprites/prop_solderpaste.png", 1);
	INIT_SPRITE(uv_sprite, "sprites/prop_uv_alt_a.png", 2);
	INIT_SPRITE(purifier_sprite, "sprites/prop_purifier_eth_a.png", 8);

	// DECO PROPS
	INIT_SPRITE(coffeemaker_sprite, "sprites/prop_coffee_empty_a.png", 14);
	INIT_SPRITE(fan_a_sprite, "sprites/prop_fan_a.png", 3);
	INIT_SPRITE(fan_b_sprite, "sprites/prop_purifier_a.png", 6);
	INIT_SPRITE(lantern_sprite, "sprites/prop_lantern_lit_a.png", 2);
	INIT_SPRITE(laptop_sprite, "sprites/prop_laptop_a.png", 4);
	INIT_SPRITE(chess_sprite, "sprites/prop_chess.png", 1);

	INIT_SPRITE(table_lg_sprite, "sprites/prop_table_xl_wood.png", 1);
	INIT_SPRITE(table_sm_sprite, "sprites/prop_table_md.png", 1);
	INIT_SPRITE(chair_wood_sprite, "sprites/prop_chair_wood.png", 4);
	INIT_SPRITE(chair_stone_sprite, "sprites/prop_chair_stone.png", 4);
	INIT_SPRITE(stool_wood_sprite, "sprites/prop_stool_wood.png", 1);
	INIT_SPRITE(stool_stone_sprite, "sprites/prop_stool_stone.png", 1);
	INIT_SPRITE(stool_gold_sprite, "sprites/prop_stool_gold.png", 1);

	INIT_SPRITE(bowl_stone_sprite, "sprites/prop_bowl_stone_2.png", 1);
	INIT_SPRITE(bowl_gold_sprite, "sprites/prop_bowl_gold.png", 1);

	INIT_SPRITE(succulent_sprite, "sprites/prop_plant_md_stem.png", 2);
	INIT_SPRITE(bush_plain_sprite, "sprites/prop_bush_lg_empty.png", 1);
	INIT_SPRITE(bush_daisy_sprite, "sprites/prop_bush_lg_daisy.png", 1);
	INIT_SPRITE(bush_lilac_sprite, "sprites/prop_bush_lg_lilac.png", 1);
	INIT_SPRITE(plant_green_sprite, "sprites/prop_plant_sapling_green.png", 1);
	INIT_SPRITE(plant_maroon_sprite, "sprites/prop_plant_sapling_maroon.png", 1);
	INIT_SPRITE(plant_purple_sprite, "sprites/prop_plant_sapling_purple.png", 1);
	INIT_SPRITE(plant_yellow_sprite, "sprites/prop_plant_sapling_yellow.png", 1);
	INIT_SPRITE(flower_vig_sprite, "sprites/interact_stat_plant_vigor.png", 6);
	INIT_SPRITE(flower_foc_sprite, "sprites/interact_stat_plant_focus.png", 6);
	INIT_SPRITE(flower_spi_sprite, "sprites/interact_stat_plant_spirit.png", 6);

	INIT_SPRITE(crystal_blue_lg_sprite, "sprites/prop_crystal_lg_shard_blue.png", 1);
	INIT_SPRITE(crystal_green_lg_sprite, "sprites/prop_crystal_lg_shard_green.png", 1);
	INIT_SPRITE(crystal_purple_lg_sprite, "sprites/prop_crystal_lg_shard_purple.png", 1);

	INIT_SPRITE(effigy_blue_sprite, "sprites/prop_effigy_blue_a.png", 4);
	INIT_SPRITE(effigy_purple_sprite, "sprites/prop_effigy_purple_a.png", 4);
	INIT_SPRITE(effigy_sea_sprite, "sprites/prop_effigy_sea_a.png", 4);

	INIT_SPRITE(poster_zk_sprite, "sprites/prop_poster_md_zk.png", 1);
	INIT_SPRITE(pixel_sprite, "sprites/prop_pixel.png", 1);
	INIT_SPRITE(padkaprop_sprite, "sprites/food_padkrakow_md.png", 1);

	// PET STATES
	INIT_SPRITE(pet_idle_sprite, "sprites/pet_unicorn_idle_complex_a.png", 4);
	INIT_SPRITE(pet_walk_sprite, "sprites/pet_unicorn_default_walk_complex_a.png", 4);

	INIT_SPRITE(pet_idle_high_vig_sprite, "sprites/pet_unicorn_wing_idle_a.png", 4);
	INIT_SPRITE(pet_walk_high_vig_sprite, "sprites/pet_unicorn_wing_walk_a.png", 4);
	INIT_SPRITE(pet_idle_high_foc_sprite, "sprites/pet_unicorn_glow_idle_a.png", 4);
	INIT_SPRITE(pet_walk_high_foc_sprite, "sprites/pet_unicorn_glow_walk_a.png", 4);
	INIT_SPRITE(pet_idle_high_spi_sprite, "sprites/pet_unicorn_shimmer_idle_a.png", 4);
	INIT_SPRITE(pet_walk_high_spi_sprite, "sprites/pet_unicorn_shimmer_walk_a.png", 4);
	INIT_SPRITE(pet_wings_out_sprite, "sprites/pet_unicorn_wing_a.png", 13);
	COPY_SPRITE(pet_wings_in_sprite, pet_wings_out_sprite, false, true);

	INIT_SPRITE(pet_idle_low_vig_sprite, "sprites/pet_unicorn_tired_a.png", 4);
	INIT_SPRITE(pet_walk_low_vig_sprite, "sprites/pet_unicorn_tired_walk_a.png", 4);
	INIT_SPRITE(pet_idle_low_foc_sprite, "sprites/pet_unicorn_messy_stink_a.png", 8);
	INIT_SPRITE(pet_walk_low_foc_sprite, "sprites/pet_unicorn_messy_walk_complex_a.png", 4);
	INIT_SPRITE(pet_idle_low_spi_sprite, "sprites/pet_unicorn_sad_idle_a.png", 4);
	INIT_SPRITE(pet_walk_low_spi_sprite, "sprites/pet_unicorn_sad_walk_a.png", 4);

	INIT_SPRITE(pet_crit_vig_in_sprite, "sprites/pet_unicorn_melt_a.png", 8);
	INIT_SPRITE(pet_crit_vig_sprite, "sprites/pet_unicorn_melt.png", 1);
	COPY_SPRITE(pet_crit_vig_out_sprite, pet_crit_vig_in_sprite, false, true);

	INIT_SPRITE(pet_crit_foc_in_sprite, "sprites/pet_unicorn_tipped_a.png", 6);
	INIT_SPRITE(pet_crit_foc_sprite, "sprites/pet_unicorn_tipped.png", 1);
	COPY_SPRITE(pet_crit_foc_out_sprite, pet_crit_foc_in_sprite, false, true);

	INIT_SPRITE(pet_crit_spi_in_sprite, "sprites/pet_unicorn_block_a.png", 15);
	INIT_SPRITE(pet_crit_spi_sprite, "sprites/pet_unicorn_block_blink_a.png", 2);
	COPY_SPRITE(pet_crit_spi_out_sprite, pet_crit_spi_in_sprite, false, true);

	// PET ACTIONS
	INIT_SPRITE(pet_eat_in_sprite, "sprites/pet_unicorn_eat_lower_a.png", 7);
	INIT_SPRITE(pet_eat_sprite, "sprites/pet_unicorn_eat_chew_a.png", 2);
	COPY_SPRITE(pet_eat_out_sprite, pet_eat_in_sprite, false, true);

	INIT_SPRITE(pet_study_in_sprite, "sprites/pet_unicorn_read_sit_a.png", 6);
	INIT_SPRITE(pet_study_sprite, "sprites/pet_unicorn_read_sit.png", 1);
	COPY_SPRITE(pet_study_out_sprite, pet_study_in_sprite, false, true);

	INIT_SPRITE(pet_play_a_sprite, "sprites/pet_unicorn_play_knead_a.png", 6);
	INIT_SPRITE(pet_play_b_sprite, "sprites/pet_unicorn_play_b_a.png", 6);
	INIT_SPRITE(pet_play_c_sprite, "sprites/pet_unicorn_play_c_a.png", 6);

	INIT_SPRITE(pet_vig_up_sprite, "sprites/pet_unicorn_stat_vigor_up_a.png", 13);
	INIT_SPRITE(pet_foc_up_sprite, "sprites/pet_unicorn_stat_focus_up_a.png", 13);
	INIT_SPRITE(pet_spi_up_sprite, "sprites/pet_unicorn_stat_spirit_up_a.png", 13);

	// PET MOODS
	INIT_SPRITE(mood_low_vig_sprite, "sprites/bubl_low_vig.png", 3);
	INIT_SPRITE(mood_low_foc_sprite, "sprites/bubl_low_foc.png", 3);
	INIT_SPRITE(mood_low_spi_sprite, "sprites/bubl_low_spi.png", 3);
	INIT_SPRITE(mood_good_sprite, "sprites/bubl_react_good.png", 5);
	INIT_SPRITE(mood_bad_sprite, "sprites/bubl_react_bad.png", 3);

	// SNAKE
	INIT_SPRITE(snake_head_sprite, "sprites/snake_cat_head_default.png", 4);
	INIT_SPRITE(snake_body_sprite, "sprites/snake_cat_body.png", 4);
	INIT_SPRITE(snake_corner_sprite, "sprites/snake_cat_corner.png", 4);
	INIT_SPRITE(snake_tail_sprite, "sprites/snake_tail.png", 4);

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
