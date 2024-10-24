#include "cat_sprite.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "png.h"

#include "cat_core.h"

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

int CAT_atlas_add(int x, int y, int w, int h)
{
	if(atlas.length >= 512)
	{
		return -1;
	}

	int idx = atlas.length;
	atlas.table[idx] = (CAT_sprite) {x, y, w, h};
	atlas.length += 1;
	return idx;
}

CAT_spriter spriter;

void CAT_spriter_init()
{
	spriter.frame = malloc(sizeof(uint16_t) * LCD_SCREEN_W * LCD_SCREEN_H);
	spriter.mode = CAT_DRAW_MODE_DEFAULT;
}

void CAT_draw_sprite(int x, int y, int key)
{
	CAT_sprite sprite = atlas.table[key];
	int w = sprite.width;
	int h = sprite.height;
	
	int x_shift = 0;
	int y_shift = 0;
	if((spriter.mode & CAT_DRAW_MODE_BOTTOM) > 0)
		y_shift = -h;
	if((spriter.mode & CAT_DRAW_MODE_CENTER_X) > 0)
		x_shift = -w/2;
	if((spriter.mode & CAT_DRAW_MODE_CENTER_Y) > 0)
		y_shift = -h/2;
	
	if((spriter.mode & CAT_DRAW_MODE_WIREFRAME) > 0)
	{
		int top = y + y_shift;
		int bottom = y + h-1 + y_shift;
		int left = x + x_shift;
		int right = x + w-1 + x_shift;
		for(int i = 0; i < 5; i++)
		{
			spriter.frame[top * LCD_SCREEN_W + left + i] = 0xF800;
			spriter.frame[top * LCD_SCREEN_W + right - i] = 0xF800;
			spriter.frame[bottom * LCD_SCREEN_W + left + i] = 0xF800;
			spriter.frame[bottom * LCD_SCREEN_W + right - i] = 0xF800;
			spriter.frame[(top + i) * LCD_SCREEN_W + x + left] = 0xF800;
			spriter.frame[(top + i) * LCD_SCREEN_W + x + right] = 0xF800;
			spriter.frame[(bottom - i) * LCD_SCREEN_W + left] = 0xF800;
			spriter.frame[(bottom - i) * LCD_SCREEN_W + right] = 0xF800;
		}
		return;
	}

	for(int dy = 0; dy < h; dy++)
	{
		for(int dx = 0; dx < w; dx++)
		{
			int x_r = sprite.x+dx;
			if((spriter.mode & CAT_DRAW_MODE_REFLECT_X) > 0)
				x_r = sprite.x+w-1-dx;
			int y_r = sprite.y+dy;
			int r_idx = y_r * atlas.width + x_r;
			if(!atlas.alpha[r_idx])
				continue;

			int x_w = x+dx+x_shift;
			int y_w = y+dy+y_shift;
			if(x_w < 0 || x_w >= LCD_SCREEN_W)
				continue;
			int w_idx = y_w * LCD_SCREEN_W + x_w;

			spriter.frame[w_idx] = atlas.rgb[r_idx];
		}
	}
}

void CAT_draw_tiles(int y_t, int h_t, int key)
{
	CAT_sprite tile = atlas.table[key];

	int start = y_t * 16;
	int end = start + h_t * 16;
	int y_w = start;
	int y_r = tile.y;

	while(y_w < end)
	{
		uint16_t* row_r = &atlas.rgb[y_r * atlas.width + tile.x];
		uint16_t* row_w = &spriter.frame[y_w * LCD_SCREEN_W];

		int x_w = 0;
		int x_r = 0;
		while(x_w < LCD_SCREEN_W)
		{
			row_w[x_w] = row_r[x_r];
			if(++x_r >= 16)
				x_r = 0;
			x_w += 1;
		}
		
		if(++y_r >= tile.y+16)
			y_r = tile.y;
		y_w += 1;
	}
}

CAT_draw_queue draw_queue;

void CAT_draw_queue_init()
{
	draw_queue.length = 0;
}

void CAT_draw_queue_add(int key, int layer, int x, int y, int mode)
{
	if(draw_queue.length >= 1024)
	{
		return;
	}

	int insert_idx = draw_queue.length;
	for(int i = 0; i < insert_idx; i++)
	{
		CAT_draw_job other = draw_queue.jobs[i];
		if(layer <= other.layer && y < other.y)
		{
			insert_idx = i;
			break;
		}
	}

	for(int i = draw_queue.length; i > insert_idx; i--)
	{
		draw_queue.jobs[i] = draw_queue.jobs[i-1];
	}
	draw_queue.jobs[insert_idx] = (CAT_draw_job) {key, layer, x, y, mode};
	draw_queue.length += 1;
}

void CAT_draw_queue_submit()
{
	for(int i = 0; i < draw_queue.length; i++)
	{
		CAT_draw_job job = draw_queue.jobs[i];
		spriter.mode = job.mode;
		CAT_draw_sprite(job.x, job.y, job.key);
	}
	draw_queue.length = 0;
}

void CAT_anim_init(CAT_anim* anim)
{
	anim->length = 0;
	anim->idx = 0;
	anim->looping = 1;
}

void CAT_anim_add(CAT_anim* anim, int key)
{
	anim->frames[anim->length] = key;
	anim->length += 1;
}

void CAT_anim_tick(CAT_anim* anim)
{
	anim->idx += 1;
	if(anim->looping && anim->idx >= anim->length)
	{
		anim->idx = 0;
	}
}

int CAT_anim_frame(CAT_anim* anim)
{
	return anim->frames[anim->idx];
}

CAT_animator animator;

void CAT_animator_init()
{
	animator.length = 0;
	animator.period = 0.2f;
	animator.timer = 0.0f;
}

void CAT_animator_add(CAT_anim* anim)
{
	if(animator.length >= 256)
	{
		return;
	}

	animator.anims[animator.length] = anim;
	animator.length += 1;
}

void CAT_animator_tick(float dt)
{
	animator.timer += dt;
	if(animator.timer >= animator.period)
	{
		for(int i = 0; i < animator.length; i++)
		{
			CAT_anim_tick(animator.anims[i]);
		}
		animator.timer = 0.0f;
	}
	animator.length = 0;
}
