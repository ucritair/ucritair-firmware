#include "cat_render.h"

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

void CAT_draw_sprite(const CAT_sprite* sprite, int frame_idx, int x, int y)
{
	if(sprite == NULL)
	{
		CAT_printf("[ERROR] CAT_draw_sprite: null sprite\n");
		return;
	}

	int w = sprite->width;
	int h = sprite->height;
	if((spriter.mode & CAT_DRAW_MODE_CENTER_X) > 0)
		x -= w/2;
	if((spriter.mode & CAT_DRAW_MODE_CENTER_Y) > 0)
		y -= h/2;
	else if((spriter.mode & CAT_DRAW_MODE_BOTTOM) > 0)
		y -= h;

#ifdef CAT_EMBEDDED
	y -= framebuffer_offset_h;
#endif
	int y_f = y + h;
	if (y >= LCD_FRAMEBUFFER_H)
		return;
	if(y_f < 0)
		return;
	if(y_f > LCD_FRAMEBUFFER_H)
		y_f = LCD_FRAMEBUFFER_H;

	const uint8_t* frame = sprite->frames[frame_idx];
	int run_idx = 0;
	int dx = 0;

	while(y < y_f)
	{
		uint8_t token = frame[run_idx++];
		uint16_t colour_idx = token == 0xff ? frame[run_idx++] : token;
		uint16_t colour_565 = sprite->colour_table[colour_idx];
#ifdef CAT_DESKTOP
		if(colour_565 != 0xdead)
			colour_565 = (colour_565 >> 8) | ((colour_565 & 0xff) << 8);
#endif
		uint8_t run_length = token == 0xff ? frame[run_idx++] : 1;
		uint8_t run_remainder = run_length;
		
		while(run_remainder > 0)
		{
			int x_w =
			((spriter.mode & CAT_DRAW_MODE_REFLECT_X) > 0) ?
			x + w - dx : x + dx;

			if
			(
				colour_565 != 0xdead &&
				y >= 0 && x_w >= 0 && x_w < LCD_SCREEN_W
			)
			{
				
				int px_idx = y * LCD_SCREEN_W + x_w;
				FRAMEBUFFER[px_idx] = colour_565;
			}
			
			dx += 1;
			if(dx >= w)
			{
				dx = 0;
				y += 1;
			}
			run_remainder -= 1;
		}
	}
}

uint16_t rle_work_region[160];
struct {
	const uint8_t* ptr;
	const uint16_t* colortab;
	int width;
	int rle_count;
	uint16_t rle_word;
} rle_decode_state;

void init_rle_decode(const CAT_sprite* sprite, int frame_idx, int width)
{
	rle_decode_state.ptr = sprite->frames[frame_idx];
	rle_decode_state.colortab = sprite->colour_table;
	rle_decode_state.width = width;
	rle_decode_state.rle_count = 0;
}

void unpack_rle_row()
{
#define RLE_NEXT() *(rle_decode_state.ptr++)
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

		if (word == 0xff)
		{
			uint16_t c = rle_decode_state.colortab[RLE_NEXT()];
			c = (c >> 8) | ((c & 0xff) << 8);
			rle_decode_state.rle_word = c;
			rle_decode_state.rle_count = RLE_NEXT();
		}
		else
		{
			uint16_t c = rle_decode_state.colortab[word];
			c = (c >> 8) | ((c & 0xff) << 8);
			rle_work_region[unpacked++] = c;
		}
	}
}

void CAT_draw_tiles(const CAT_sprite* sprite, int frame_idx, int y_t, int h_t)
{
	if(sprite == NULL)
	{
		CAT_printf("[ERROR] CAT_draw_tiles: null sprite\n");
		return;
	}

	int y_start = y_t * CAT_TILE_SIZE;
	int y_end = (y_t+h_t) * CAT_TILE_SIZE;
#ifdef CAT_EMBEDDED
	y_start -= framebuffer_offset_h;
	y_end -= framebuffer_offset_h;
#endif

	if (y_start >= LCD_FRAMEBUFFER_H) return;
	if (y_end < 0) return;

	if (y_start < 0) y_start = 0;
	if (y_end > LCD_FRAMEBUFFER_H) y_end = LCD_FRAMEBUFFER_H;

	init_rle_decode(sprite, frame_idx, sprite->width);

#define RESETPTR from = (uint32_t*)rle_work_region;
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
		unpack_rle_row();
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