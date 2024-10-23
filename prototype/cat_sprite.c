#include "cat_sprite.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "png.h"

CAT_atlas atlas;

void CAT_atlas_init(const char* path)
{
	FILE* file = fopen(path, "rb");

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
	png_get_IHDR(png, info, &width, &height, NULL, NULL, NULL, NULL, NULL);\
	atlas.width = width;
	atlas.height = height;
	
	size_t row_size = png_get_rowbytes(png, info);
	size_t size = row_size * height;
	uint8_t bytes[size];
	png_bytepp rows = png_get_rows(png, info);
	for(int i = 0; i < height; i++)
	{
		memcpy(bytes + row_size * i, rows[i], row_size);
	}

	atlas.rgb = malloc(sizeof(uint16_t) * width * height);
	atlas.alpha = malloc(sizeof(bool) * width * height);
	for(int i = 0; i < width*height; i++)
	{
		uint8_t r_8 = bytes[i*4+0];
		uint8_t g_8 = bytes[i*4+1];
		uint8_t b_8 = bytes[i*4+2];
		uint8_t a_8 = bytes[i*4+3];
		float r_n = (float) r_8 / 255.0f;
		float g_n = (float) g_8 / 255.0f;
		float b_n = (float) b_8 / 255.0f;
		uint8_t r_5 = r_n * 31;
		uint8_t g_6 = g_n * 63;
		uint8_t b_5 = b_n * 31;
		uint16_t rgb_565 = (r_5 << 11) | (g_6 << 5) | b_5;
		atlas.rgb[i] = rgb_565;
		atlas.alpha[i] = a_8 >= 255;
	}

	png_destroy_read_struct(&png, &info, NULL);

	for(int i = 0; i < 512; i++)
	{
		atlas.table[i] = (CAT_sprite) {0, 0, 0, 0};
	}
	atlas.length = 0;
}

int CAT_atlas_add(CAT_sprite sprite)
{
	if(atlas.length >= 512)
	{
		return -1;
	}

	int idx = atlas.length;
	atlas.table[idx] = sprite;
	atlas.length += 1;
	return idx;
}

CAT_spriter spriter;

void CAT_spriter_init()
{
	spriter.frame = malloc(sizeof(uint16_t) * 240 * 320);
	spriter.mode = CAT_DRAW_MODE_DEFAULT;
}

void CAT_draw_sprite(int x, int y, int key)
{
	CAT_sprite sprite = atlas.table[key];
	
	int x_shift = 0;
	int y_shift = 0;
	if((spriter.mode & CAT_DRAW_MODE_BOTTOM) > 0)
		y_shift = -sprite.height;
	if((spriter.mode & CAT_DRAW_MODE_CENTER_X) > 0)
		x_shift = -sprite.width/2;
	if((spriter.mode & CAT_DRAW_MODE_CENTER_Y) > 0)
		y_shift = -sprite.height/2;

	for(int dy = 0; dy < sprite.height; dy++)
	{
		for(int dx = 0; dx < sprite.width; dx++)
		{
			int x_r = sprite.x+dx;
			if((spriter.mode & CAT_DRAW_MODE_REFLECT_X) > 0)
				x_r = sprite.x+sprite.width-1-dx;
			int y_r = sprite.y+dy;
			int r_idx = y_r * atlas.width + x_r;
			if(!atlas.alpha[r_idx])
				continue;

			int x_w = x+dx+x_shift;
			int y_w = y+dy+y_shift;
			int w_idx = y_w * 240 + x_w;

			spriter.frame[w_idx] = atlas.rgb[r_idx];
		}
	}
}

void CAT_anim_init(CAT_anim* anim)
{
	anim->frame_count = 0;
	anim->idx = 0;
	anim->looping = 1;
}

void CAT_anim_add(CAT_anim* anim, int key)
{
	anim->frames[anim->frame_count] = key;
	anim->frame_count += 1;
}

void CAT_anim_tick(CAT_anim* anim)
{
	anim->idx += 1;
	if(anim->looping && anim->idx >= anim->frame_count)
	{
		anim->idx = 0;
	}
}

void CAT_anim_cmd_init(CAT_anim_cmd* cmd, CAT_anim* anim, int layer, int x, int y, int mode)
{
	cmd->anim = anim;
	cmd->layer = layer;
	cmd->x = x;
	cmd->y = y;
	cmd->mode = mode;
}

CAT_anim_queue anim_queue;

void CAT_anim_queue_init()
{
	anim_queue.length = 0;
	anim_queue.period = 0.2;
	anim_queue.timer = 0;
}

void CAT_anim_queue_add(CAT_anim_cmd cmd)
{
	if(anim_queue.length >= 1024)
	{
		return;
	}

	int insert_idx = anim_queue.length;
	for(int i = 0; i < insert_idx; i++)
	{
		CAT_anim_cmd other = anim_queue.items[i];
		if(cmd.layer <= other.layer && cmd.y < other.y)
		{
			insert_idx = i;
			break;
		}
	}

	for(int i = anim_queue.length; i > insert_idx; i--)
	{
		anim_queue.items[i] = anim_queue.items[i-1];
	}
	anim_queue.items[insert_idx] = cmd;
	anim_queue.length += 1;
}

void CAT_anim_queue_tick(float dt)
{
	anim_queue.timer += dt;
	if(anim_queue.timer >= anim_queue.period)
	{
		for(int i = 0; i < anim_queue.length; i++)
		{
			CAT_anim_cmd cmd = anim_queue.items[i];
			CAT_anim* anim = cmd.anim;
			CAT_anim_tick(anim);
		}
		anim_queue.timer = 0;
	}
}

void CAT_anim_queue_draw()
{
	for(int i = 0; i < anim_queue.length; i++)
	{
		CAT_anim_cmd cmd = anim_queue.items[i];
		CAT_anim* anim = cmd.anim;

		spriter.mode = cmd.mode;
		CAT_draw_sprite
		(
			cmd.x, cmd.y,
			anim->frames[anim->idx]
		);
	}
	anim_queue.length = 0;
}
