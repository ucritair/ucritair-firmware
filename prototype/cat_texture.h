#ifndef CAT_TEXTURE_H
#define CAT_TEXTURE_H

#include "png.h"

#include "cowtools.h"

typedef struct CAT_colour
{
	float r;
	float g;
	float b;
	float a;
} CAT_colour;

CAT_colour CAT_colour_lerp(CAT_colour a, CAT_colour b, float t)
{
	CAT_colour c;
	c.r = lerp(a.r, b.r, t);
	c.g = lerp(a.g, b.g, t);
	c.b = lerp(a.b, b.b, t);
	c.a = lerp(a.a, b.a, t);
	return c;
}

typedef struct CAT_texture
{
	int width;
	int height;
	float* data;
} CAT_texture;

void CAT_texture_init(CAT_texture* texture, int width, int height)
{
	texture->width = width;
	texture->height = height;
	texture->data = (float*) malloc(width * height * 4 * sizeof(float));
}

CAT_colour CAT_texture_read(CAT_texture* texture, int x, int y)
{
	int idx = (y * texture->width + x) * 4;
	CAT_colour c;
	c.r = texture->data[idx+0];
	c.g = texture->data[idx+1];
	c.b = texture->data[idx+2];
	c.a = texture->data[idx+3];
	return c;
}

void CAT_texture_write(CAT_texture* texture, int x, int y, CAT_colour c)
{
	CAT_colour base = CAT_texture_read(texture, x, y);
	CAT_colour blend = CAT_colour_lerp(base, c, c.a);
	int idx = (y * texture->width + x) * 4;
	texture->data[idx+0] = blend.r;
	texture->data[idx+1] = blend.g;
	texture->data[idx+2] = blend.b;
	texture->data[idx+3] = blend.a;
}

void CAT_load_PNG(CAT_texture* texture, const char* path)
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
	
	size_t row_size = png_get_rowbytes(png, info);
	size_t size = row_size * height;
	uint8_t bytes[size];
	
	png_bytepp rows = png_get_rows(png, info);
	for(int i = 0; i < height; i++)
	{
		memcpy(bytes + row_size * i, rows[i], row_size);
	}

	CAT_texture_init(texture, width, height);
	for(int y = 0; y < height*4; y++)
	{
		for(int x = 0; x < width*4; x++)
		{
			int idx = y * width + x;
			uint8_t c_b = bytes[idx];
			float c_f = c_b / 255.0f;
			texture->data[idx] = c_f;
		}
	}

	png_destroy_read_struct(&png, &info, NULL);
}

//TODO handle alignment and positioning modes

typedef struct CAT_sprite
{
	int x;
	int y;
	int width;
	int height;
} CAT_sprite;

typedef struct CAT_atlas
{
	CAT_texture* texture;
	CAT_sprite map[256];
} CAT_atlas;

void CAT_atlas_register(CAT_atlas* atlas, int key, CAT_sprite sprite)
{
	atlas->map[key] = sprite;
}

typedef struct CAT_animation
{
	int frame_count;
	int keys[24];
	int idx;
	int looping;
} CAT_animation;

void CAT_animation_init(CAT_animation* anim)
{
	anim->frame_count = 0;
	anim->idx = 0;
	anim->looping = 1;
}

void CAT_animation_register(CAT_animation* anim, int key)
{
	anim->keys[anim->frame_count] = key;
	anim->frame_count += 1;
}

void CAT_animation_tick(CAT_animation* anim)
{
	anim->idx += 1;
	if(anim->looping && anim->idx >= anim->frame_count)
	{
		anim->idx = 0;
	}
}

int CAT_animation_frame(CAT_animation* anim)
{
	return anim->keys[anim->idx];
}

void CAT_draw_sprite(CAT_texture* frame, int x, int y, CAT_atlas* atlas, int key, int mode)
{
	CAT_sprite sprite = atlas->map[key];
	
	int x_shift = 0;
	int y_shift = 0;
	if(mode == 1)
	{
		x_shift = -sprite.width/2;
		y_shift = -sprite.height;
	}

	for(int dy = 0; dy < sprite.height; dy++)
	{
		for(int dx = 0; dx < sprite.width; dx++)
		{
			int x_r = sprite.x+dx;
			int y_r = sprite.y+dy;
			CAT_colour c = CAT_texture_read(atlas->texture, x_r, y_r);
			int x_w = x+dx+x_shift;
			int y_w = y+dy+y_shift;
			CAT_texture_write(frame, x_w, y_w, c);
		}
	}
}

#endif
