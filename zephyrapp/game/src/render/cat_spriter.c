#include "cat_render.h"

#include "sprite_assets.h"

//////////////////////////////////////////////////////////////////////////
// SPRITER

CAT_draw_flag draw_flags = CAT_DRAW_FLAG_DEFAULT;

void CAT_draw_sprite(const CAT_sprite* sprite, int frame_idx, int x, int y)
{
	if(sprite == NULL)
	{
		CAT_printf("[WARNING] CAT_draw_sprite: null sprite\n");
		sprite = &null_sprite;
		frame_idx = 0;
	}
	else if(frame_idx < 0 || frame_idx >= sprite->frame_count)
	{
		CAT_printf("[WARNING] CAT_draw_sprite: frame index %d out of bounds\n", frame_idx);
		frame_idx = sprite->frame_count-1;
	}

	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	int w = sprite->width;
	int h = sprite->height;
	if ((draw_flags & CAT_DRAW_FLAG_CENTER_X) > 0)
		x -= w / 2;
	if ((draw_flags & CAT_DRAW_FLAG_CENTER_Y) > 0)
		y -= h / 2;
	else if ((draw_flags & CAT_DRAW_FLAG_BOTTOM) > 0)
		y -= h;
	int y_i = y;

	y -= FRAMEBUFFER_ROW_OFFSET;
	int y_f = y + h;
	if (y >= CAT_LCD_FRAMEBUFFER_H)
		return;
	if (y_f < 0)
		return;
	if (y_f > CAT_LCD_FRAMEBUFFER_H)
		y_f = CAT_LCD_FRAMEBUFFER_H;

	const uint8_t* frame = sprite->frames[frame_idx];
	int run_idx = 0;
	int dx = 0;
	bool valid_draw_region = true;

	while(valid_draw_region)
	{
		uint8_t token = frame[run_idx++];
		uint16_t colour_idx = token == 0xff ? frame[run_idx++] : token;
		uint16_t colour_565 = sprite->colour_table[colour_idx];
		if(colour_565 != 0xdead)
			colour_565 = ADAPT_EMBEDDED_COLOUR(colour_565);
		uint8_t run_length = token == 0xff ? frame[run_idx++] : 1;
		uint8_t run_remainder = run_length;
		
		while(run_remainder > 0)
		{
			int x_w =
			((draw_flags & CAT_DRAW_FLAG_REFLECT_X) > 0) ?
			x + w - dx : x + dx;

			if
			(
				colour_565 != 0xdead &&
				y >= 0 && x_w >= 0 && x_w < CAT_LCD_FRAMEBUFFER_W
			)
			{
				int px_idx = y * CAT_LCD_FRAMEBUFFER_W + x_w;
				framebuffer[px_idx] = colour_565;
			}
			
			dx += 1;
			if (dx >= w)
			{
				dx = 0;
				y += 1;
				if(y >= y_f)
				{
					valid_draw_region = false;
					break;
				}
			}
			run_remainder -= 1;
		}
	}
}

uint16_t rle_work_region[160];
struct
{
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
			c = ADAPT_EMBEDDED_COLOUR(c);
			rle_decode_state.rle_word = c;
			rle_decode_state.rle_count = RLE_NEXT();
		}
		else
		{
			uint16_t c = rle_decode_state.colortab[word];
			c = ADAPT_EMBEDDED_COLOUR(c);
			rle_work_region[unpacked++] = c;
		}
	}
}