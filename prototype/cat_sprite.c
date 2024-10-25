#include "cat_sprite.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "png.h"

#include "cat_core.h"

//////////////////////////////////////////////////////////////////////////
// ATLAS AND SPRITER

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

	atlas.length = 0;
}

int CAT_atlas_add(int x, int y, int w, int h)
{
	if(atlas.length >= CAT_ATLAS_MAX_LENGTH)
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

void CAT_draw_sprite(int x, int y, int sprite_id)
{
	CAT_sprite sprite = atlas.table[sprite_id];
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

void CAT_draw_tiles(int y_t, int h_t, int sprite_id)
{
	CAT_sprite tile = atlas.table[sprite_id];

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

//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

CAT_draw_queue draw_queue;

void CAT_draw_queue_init()
{
	draw_queue.length = 0;
}

void CAT_draw_queue_add(int sprite_id, int layer, int x, int y, int mode)
{
	if(draw_queue.length >= CAT_DRAW_QUEUE_MAX_LENGTH)
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
	draw_queue.jobs[insert_idx] = (CAT_draw_job) {sprite_id, layer, x, y, mode};
	draw_queue.length += 1;
}

void CAT_draw_queue_submit()
{
	for(int i = 0; i < draw_queue.length; i++)
	{
		CAT_draw_job job = draw_queue.jobs[i];
		spriter.mode = job.mode;
		CAT_draw_sprite(job.x, job.y, job.sprite_id);
	}
	draw_queue.length = 0;
}

//////////////////////////////////////////////////////////////////////////
// ANIMATIONS

CAT_anim_table anim_table;

void CAT_anim_table_init()
{
	anim_table.length = 0;
}

int CAT_anim_init()
{
	if(anim_table.length >= CAT_ANIM_TABLE_MAX_LENGTH)
		return -1;

	int anim_id = anim_table.length;
	anim_table.length += 1;

	CAT_anim* anim = &anim_table.data[anim_id];
	anim->length = 0;
	anim->idx = 0;
	anim->looping = 1;

	return anim_id;
}

CAT_anim* CAT_anim_get(int anim_id)
{
	if(anim_id < 0 || anim_id >= CAT_ANIM_TABLE_MAX_LENGTH)
		return NULL;
	return &anim_table.data[anim_id];
}

void CAT_anim_add(int anim_id, int sprite_id)
{
	CAT_anim* anim = CAT_anim_get(anim_id);
	if(anim->length >= CAT_ANIM_MAX_LENGTH)
		return;
	anim->frames[anim->length] = sprite_id;
	anim->length += 1;
}

void CAT_anim_tick(int anim_id)
{
	CAT_anim* anim = CAT_anim_get(anim_id);
	anim->idx += 1;
	if(anim->looping && anim->idx >= anim->length)
	{
		anim->idx = 0;
	}
}

int CAT_anim_frame(int anim_id)
{
	CAT_anim* anim = CAT_anim_get(anim_id);
	return anim->frames[anim->idx];
}

//////////////////////////////////////////////////////////////////////////
// ANIMATOR

CAT_animator animator;

void CAT_animator_init()
{
	animator.length = 0;
	animator.period = 0.2f;
	animator.timer = 0.0f;
}

void CAT_animator_add(int anim_id, int layer, int x, int y, int mode)
{
	if(animator.length >= CAT_ANIMATOR_MAX_LENGTH)
	{
		return;
	}

	animator.jobs[animator.length] = (CAT_anim_job) {anim_id, layer, x, y, mode};
	animator.length += 1;
}

void CAT_animator_submit()
{
	animator.timer += simulator.delta_time;
	if(animator.timer >= animator.period)
	{
		for(int i = 0; i < animator.length; i++)
		{
			CAT_anim_tick(animator.jobs[i].anim_id);
		}
		animator.timer = 0.0f;
	}
	for(int i = 0; i < animator.length; i++)
	{
		CAT_anim_job job = animator.jobs[i];
		int sprite_id = CAT_anim_frame(job.anim_id);
		CAT_draw_queue_add(sprite_id, job.layer, job.x, job.y, job.mode);
	}
	animator.length = 0;
}

//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

int wall_sprite_id[3];
int floor_sprite_id[3];
int pet_sprite_id[13];
int vending_sprite_id[13];
int pot_sprite_id[7];
int chair_sprite_id[4];
int table_sprite_id;
int coffee_sprite_id[2];
int device_sprite_id;

int cursor_sprite_id[4];
int vigor_sprite_id;
int focus_sprite_id;
int soul_sprite_id;
int ring_hl_sprite_id;

int panel_sprite_id[9];
int glyph_sprite_id[91];
int a_sprite_id;
int b_sprite_id;
int enter_sprite_id;
int exit_sprite_id;
int select_sprite_id;
int arrow_sprite_id;
int item_sprite_id;

int idle_anim_id;
int walk_anim_id;
int mood_anim_id;
int vending_anim_id;
int pot_anim_id;
int chair_anim_id;
int coffee_anim_id;

int cursor_anim_id;

void CAT_sprite_mass_define()
{
	for(int i = 0; i < 3; i++)
	{
		wall_sprite_id[i] = CAT_atlas_add(16*i, 0, 16, 16);
		floor_sprite_id[i] = CAT_atlas_add(16*i, 16, 16, 16);
	}
	for(int i = 0; i < 13; i++)
	{
		pet_sprite_id[i] = CAT_atlas_add(64*i, 544, 64, 48);
	}
	for(int i = 0; i < 13; i++)
	{
		vending_sprite_id[i] = CAT_atlas_add(64*i, 448, 64, 96);
	}
	for(int i = 0; i < 7; i++)
	{
		pot_sprite_id[i] = CAT_atlas_add(32*i, 240, 32, 64);
	}
	for(int i = 0; i < 4; i++)
	{
		chair_sprite_id[i] = CAT_atlas_add(32*i, 48, 32, 48);
	}
	table_sprite_id = CAT_atlas_add(128, 48, 64, 48);
	for(int i = 0; i < 2; i++)
	{
		coffee_sprite_id[i] = CAT_atlas_add(192+32*i, 48, 32, 48);
	}
	device_sprite_id = CAT_atlas_add(256, 48, 48, 48);

	for(int i = 0; i < 4; i++)
	{
		cursor_sprite_id[i] = CAT_atlas_add(16*i, 336, 16, 16);
	}
	vigor_sprite_id = CAT_atlas_add(128, 0, 32, 32);
	focus_sprite_id = CAT_atlas_add(160, 0, 32, 32);
	soul_sprite_id = CAT_atlas_add(192, 0, 32, 32);
	ring_hl_sprite_id = CAT_atlas_add(224, 0, 32, 32);

	for(int i = 0; i < 3; i++)
	{
		panel_sprite_id[0+i] = CAT_atlas_add(48+16*i, 0, 16, 16);
		panel_sprite_id[3+i] = CAT_atlas_add(48+16*i, 16, 16, 16);
		panel_sprite_id[6+i] = CAT_atlas_add(48+16*i, 32, 16, 16);
	}
	for(int i = 0; i < 91; i++)
	{
		glyph_sprite_id[i] = CAT_atlas_add(8*i, 592, 8, 12);
	}
	a_sprite_id = CAT_atlas_add(96, 0, 16, 16);
	b_sprite_id = CAT_atlas_add(112, 0, 16, 16);
	enter_sprite_id = CAT_atlas_add(96, 16, 16, 16);
	exit_sprite_id = CAT_atlas_add(112, 16, 16, 16);
	select_sprite_id = CAT_atlas_add(96, 32, 16, 16);
	arrow_sprite_id = CAT_atlas_add(112, 32, 16, 16);
	item_sprite_id = CAT_atlas_add(192, 304, 16, 16);
	
	idle_anim_id = CAT_anim_init();
	for(int i = 0; i < 2; i++)
	{
		CAT_anim_add(idle_anim_id, pet_sprite_id[i]); 
	}
	walk_anim_id = CAT_anim_init();
	for(int i = 0; i < 2; i++)
	{
		CAT_anim_add(walk_anim_id, pet_sprite_id[2+i]); 
	}
	mood_anim_id = CAT_anim_init();
	for(int i = 0; i < 9; i++)
	{
		CAT_anim_add(mood_anim_id, pet_sprite_id[4+i]); 
	}
	vending_anim_id = CAT_anim_init();
	for(int i = 0; i < 13; i++)
	{
		CAT_anim_add(vending_anim_id, vending_sprite_id[i]); 
	}
	pot_anim_id = CAT_anim_init();
	for(int i = 0; i < 7; i++)
	{
		CAT_anim_add(pot_anim_id, pot_sprite_id[i]);
	}
	chair_anim_id = CAT_anim_init();
	for(int i = 0; i < 4; i++)
	{
		CAT_anim_add(chair_anim_id, chair_sprite_id[i]); 
	}
	coffee_anim_id = CAT_anim_init();
	for(int i = 0; i < 2; i++)
	{
		CAT_anim_add(coffee_anim_id, coffee_sprite_id[i]); 
	}

	cursor_anim_id = CAT_anim_init();
	for(int i = 0; i < 4; i++)
	{
		CAT_anim_add(cursor_anim_id, cursor_sprite_id[i]); 
	}
}
