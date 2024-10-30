#include "cat_sprite.h"

#include <stdint.h>
#include <string.h>

#include "cat_core.h"

//////////////////////////////////////////////////////////////////////////
// ATLAS AND SPRITER

CAT_atlas atlas;

#ifndef CAT_BAKED_ASSETS

#include "png.h"

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
	png_destroy_read_struct(&png, &info, NULL);

	atlas.rgb = CAT_malloc(sizeof(uint16_t) * width * height);
	atlas.alpha = CAT_malloc(sizeof(bool) * width * height);
	for(int i = 0; i < width*height; i++)
	{
		uint8_t r_8 = bytes[i*4];
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
	atlas.length = 0;
}

#else

#ifndef CAT_DESKTOP
extern uint16_t* image_data_table[];
#else
#include "../../script/images.c"
#endif

extern uint16_t lcd_framebuffer[];

void CAT_atlas_init(const char* path)
{
	atlas.length = 0;
}

#endif

int CAT_atlas_add(int x, int y, int w, int h)
{
	if(atlas.length >= CAT_ATLAS_MAX_LENGTH)
	{
		return -1;
	}

	int idx = atlas.length;
	atlas.table[idx] = (CAT_sprite) {
#ifndef CAT_BAKED_ASSETS
		x, y,
#endif
		w, h
	};
	atlas.length += 1;
	return idx;
}

void CAT_atlas_cleanup()
{
#ifndef CAT_BAKED_ASSETS
	atlas.width = 0;
	atlas.height = 0;
	CAT_free(atlas.rgb);
	CAT_free(atlas.alpha);
	atlas.rgb = NULL;
	atlas.alpha = NULL;
#endif
}

CAT_spriter spriter;

void CAT_spriter_init()
{
#ifdef CAT_DESKTOP
	spriter.frame = CAT_malloc(sizeof(uint16_t) * LCD_SCREEN_W * LCD_SCREEN_H);
#else
	spriter.frame = lcd_framebuffer;
#endif
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

#ifndef CAT_BAKED_ASSETS
			int x_r = sprite.x+dx;
			if((spriter.mode & CAT_DRAW_MODE_REFLECT_X) > 0)
				x_r = sprite.x+w-1-dx;
			int y_r = sprite.y+dy;
			int r_idx = y_r * atlas.width + x_r;
			uint16_t px = *(atlas.rgb + r_idx);
			if(!atlas.alpha[r_idx])
				continue;
#else
			int x_r = dx;
			if (spriter.mode & CAT_DRAW_MODE_REFLECT_X)
				x_r = w-1-dx;
			uint16_t px = *(image_data_table[sprite_id] + (w * dy) + x_r);
			if (px == 0xdead)
				continue;
#endif

			int x_w = x+dx+x_shift;
			int y_w = y+dy+y_shift;
			if(x_w < 0 || x_w >= LCD_SCREEN_W)
				continue;
			int w_idx = y_w * LCD_SCREEN_W + x_w;

			spriter.frame[w_idx] = px;
		}
	}
}

void CAT_draw_tiles(int y_t, int h_t, int sprite_id)
{
	CAT_sprite tile = atlas.table[sprite_id];

	int start = y_t * CAT_TILE_SIZE;
	int end = start + h_t * CAT_TILE_SIZE;
	int y_w = start;
	int y_r = 0;

	while(y_w < end)
	{
#ifndef CAT_BAKED_ASSETS
		uint16_t* row_r = &atlas.rgb[(tile.y + y_r) * atlas.width + tile.x];
#else
		uint16_t* row_r = image_data_table[sprite_id] + (y_r*tile.width);
#endif
		uint16_t* row_w = &spriter.frame[y_w * LCD_SCREEN_W];

		int x_w = 0;
		int x_r = 0;
		while(x_w < LCD_SCREEN_W)
		{
			row_w[x_w] = row_r[x_r];
			if(++x_r >= CAT_TILE_SIZE)
				x_r = 0;
			x_w += 1;
		}
		
		if(++y_r >= CAT_TILE_SIZE)
			y_r = 0;
		y_w += 1;
	}
}

void CAT_spriter_cleanup()
{
	CAT_free(spriter.frame);
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
		if(layer < other.layer || y < other.y)
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

int CAT_anim_init(int* sprite_id, int length, bool looping)
{
	if(anim_table.length >= CAT_ANIM_TABLE_MAX_LENGTH)
		return -1;

	int anim_id = anim_table.length;
	anim_table.length += 1;

	CAT_anim* anim = &anim_table.data[anim_id];
	for(int i = 0; i < length; i++)
	{
		anim->frames[i] = sprite_id[i];
	}
	anim->length = length;
	anim->idx = 0;
	anim->looping = looping;

	return anim_id;
}

CAT_anim* CAT_anim_get(int anim_id)
{
	if(anim_id < 0 || anim_id >= CAT_ANIM_TABLE_MAX_LENGTH)
		return NULL;
	return &anim_table.data[anim_id];
}

void CAT_anim_tick(int anim_id)
{
	CAT_anim* anim = CAT_anim_get(anim_id);
	if(anim->idx < anim->length-1)
	{
		anim->idx += 1;
	}
	else
	{
		if(anim->looping)
			anim->idx = 0;
	}
}

int CAT_anim_frame(int anim_id)
{
	CAT_anim* anim = CAT_anim_get(anim_id);
	int sprite_id = anim->frames[anim->idx];
	return sprite_id;
}

//////////////////////////////////////////////////////////////////////////
// ANIM QUEUE

CAT_anim_queue anim_queue;

void CAT_anim_queue_init()
{
	anim_queue.length = 0;
	anim_queue.period = 0.2f;
	anim_queue.timer = 0.0f;
}

void CAT_anim_queue_add(int anim_id, int layer, int x, int y, int mode)
{
	if(anim_queue.length >= CAT_ANIM_QUEUE_MAX_LENGTH)
	{
		return;
	}

	anim_queue.jobs[anim_queue.length] = (CAT_anim_job) {anim_id, layer, x, y, mode};
	anim_queue.length += 1;
}

void CAT_anim_queue_submit()
{
	anim_queue.timer += CAT_get_delta_time();
	if(anim_queue.timer >= anim_queue.period)
	{
		for(int i = 0; i < anim_queue.length; i++)
		{
			CAT_anim_tick(anim_queue.jobs[i].anim_id);
		}
		anim_queue.timer = 0.0f;
	}
	for(int i = 0; i < anim_queue.length; i++)
	{
		CAT_anim_job job = anim_queue.jobs[i];
		int sprite_id = CAT_anim_frame(job.anim_id);
		CAT_draw_queue_add(sprite_id, job.layer, job.x, job.y, job.mode);
	}
	anim_queue.length = 0;
}

//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

int wall_sprite_id[3];
int floor_sprite_id[3];
int pet_sprite_id[13];
int fed_sprite_id[10];
int studied_sprite_id[10];
int played_sprite_id[10];
int low_vigour_sprite_id[3];
int low_spirit_sprite_id[3];
int anger_sprite_id[3];
int vending_sprite_id[13];
int pot_sprite_id[7];
int chair_sprite_id[4];
int table_sprite_id;
int coffee_sprite_id[2];
int device_sprite_id;
int window_sprite_id; 
int seed_sprite_id[6];

int cursor_sprite_id[4];
int feed_button_sprite_id;
int study_button_sprite_id;
int play_button_sprite_id;
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
int cell_sprite_id[4];
int vigour_sprite_id;
int focus_sprite_id;
int spirit_sprite_id;

int idle_anim_id;
int walk_anim_id;
int death_anim_id;
int fed_anim_id;
int studied_anim_id;
int played_anim_id;
int low_vigour_anim_id;
int low_spirit_anim_id;
int anger_anim_id;
int vending_anim_id;
int pot_anim_id;
int chair_anim_id;
int coffee_anim_id;

int cursor_anim_id;

#ifdef CAT_DESKTOP
#include <stdio.h>
#endif

#ifndef CAT_DESKTOP
#define PRINT_ON_DESKTOP(...)
#else
#define PRINT_ON_DESKTOP printf
#endif

#define ATLAS_ADD(lhs, x, y, w, h) lhs = CAT_atlas_add(x, y, w, h); PRINT_ON_DESKTOP("ATLAS_ADD, " #lhs ", %d, %d, %d, %d, %d\n", x, y, w, h, lhs);

void CAT_sprite_mass_define()
{
	for(int i = 0; i < 3; i++)
	{
		ATLAS_ADD(wall_sprite_id[i], 16*i, 0, 16, 16);
		ATLAS_ADD(floor_sprite_id[i], 16*i, 16, 16, 16);
	}
	for(int i = 0; i < 13; i++)
	{
		ATLAS_ADD(pet_sprite_id[i], 64*i, 544, 64, 48);
	}
	for(int i = 0; i < 10; i++)
	{
		ATLAS_ADD(fed_sprite_id[i], 256+21*i, 0, 21, 16);
		ATLAS_ADD(studied_sprite_id[i], 256+21*i, 16, 21, 16);
		ATLAS_ADD(played_sprite_id[i], 256+21*i, 32, 21, 16);
	}
	for(int i = 0; i < 3; i++)
	{
		ATLAS_ADD(low_vigour_sprite_id[i], 466+21*i, 0, 21, 21);
		ATLAS_ADD(low_spirit_sprite_id[i], 466+21*i, 21, 21, 21);
		ATLAS_ADD(anger_sprite_id[i], 299+21*i, 64, 21, 16);
	}
	for(int i = 0; i < 13; i++)
	{
		ATLAS_ADD(vending_sprite_id[i], 64*i, 448, 64, 96);
	}
	for(int i = 0; i < 7; i++)
	{
		ATLAS_ADD(pot_sprite_id[i], 32*i, 240, 32, 64);
	}
	for(int i = 0; i < 4; i++)
	{
		ATLAS_ADD(chair_sprite_id[i], 32*i, 48, 32, 48);
	}
	ATLAS_ADD(table_sprite_id, 128, 48, 64, 48);
	for(int i = 0; i < 2; i++)
	{
		ATLAS_ADD(coffee_sprite_id[i], 192+32*i, 48, 32, 48);
	}
	ATLAS_ADD(device_sprite_id, 256, 48, 43, 48);
	ATLAS_ADD(window_sprite_id, 0, 96, 112, 61);
	for(int i = 0; i < 3; i++)
	{
		ATLAS_ADD(seed_sprite_id[0+i], 192+16*i, 304, 16, 16);
		ATLAS_ADD(seed_sprite_id[3+i], 192+16*i, 320, 16, 16);
	}

	for(int i = 0; i < 4; i++)
	{
		ATLAS_ADD(cursor_sprite_id[i], 288+16*i, 336, 16, 16);
	}
	ATLAS_ADD(feed_button_sprite_id, 128, 0, 32, 32);
	ATLAS_ADD(study_button_sprite_id, 160, 0, 32, 32);
	ATLAS_ADD(play_button_sprite_id, 192, 0, 32, 32);
	ATLAS_ADD(ring_hl_sprite_id, 224, 0, 32, 32);

	for(int i = 0; i < 3; i++)
	{
		ATLAS_ADD(panel_sprite_id[0+i], 48+16*i, 0, 16, 16);
		ATLAS_ADD(panel_sprite_id[3+i], 48+16*i, 16, 16, 16);
		ATLAS_ADD(panel_sprite_id[6+i], 48+16*i, 32, 16, 16);
	}
	for(int i = 0; i < 91; i++)
	{
		ATLAS_ADD(glyph_sprite_id[i], 8*i, 592, 8, 12);
	}
	ATLAS_ADD(a_sprite_id, 96, 0, 16, 16);
	ATLAS_ADD(b_sprite_id, 112, 0, 16, 16);
	ATLAS_ADD(enter_sprite_id, 96, 16, 16, 16);
	ATLAS_ADD(exit_sprite_id, 112, 16, 16, 16);
	ATLAS_ADD(select_sprite_id, 96, 32, 16, 16);
	ATLAS_ADD(arrow_sprite_id, 112, 32, 16, 16);
	ATLAS_ADD(item_sprite_id, 128, 32, 16, 16);
	for(int i = 0; i < 4; i++)
	{
		ATLAS_ADD(cell_sprite_id[i], 144+8*i, 32, 8, 16);
	}
	ATLAS_ADD(vigour_sprite_id, 160, 160, 22, 24);
	ATLAS_ADD(focus_sprite_id, 206, 160, 22, 22);
	ATLAS_ADD(spirit_sprite_id, 182, 160, 24, 22);
	
	idle_anim_id = CAT_anim_init(pet_sprite_id, 2, true);
	walk_anim_id = CAT_anim_init(pet_sprite_id+2, 2, true); 
	death_anim_id = CAT_anim_init(pet_sprite_id+4, 9, false); 
	fed_anim_id = CAT_anim_init(fed_sprite_id, 10, true);
	studied_anim_id = CAT_anim_init(studied_sprite_id, 10, true);
	played_anim_id = CAT_anim_init(played_sprite_id, 10, true);
	low_vigour_anim_id = CAT_anim_init(low_vigour_sprite_id, 3, true);
	low_spirit_anim_id = CAT_anim_init(low_spirit_sprite_id, 3, true);
	anger_anim_id = CAT_anim_init(anger_sprite_id, 3, true);

	vending_anim_id = CAT_anim_init(vending_sprite_id, 13, true);
	pot_anim_id = CAT_anim_init(pot_sprite_id, 7, true);
	chair_anim_id = CAT_anim_init(chair_sprite_id, 4, true);
	coffee_anim_id = CAT_anim_init(coffee_sprite_id, 2, true);

	cursor_anim_id = CAT_anim_init(cursor_sprite_id, 4, true);

#ifdef CAT_DESKTOP
#ifdef CAT_BAKED_ASSETS
#define DIAG_PRINT(i) printf(#i " = @%x=[", image_data_table[i]);\
	for (int p = 0; p < 32; p++) printf("%04x ", image_data_table[i][p]);\
		printf("]\n");

	DIAG_PRINT(vigour_sprite_id);
	DIAG_PRINT(focus_sprite_id);	
#endif
#endif
}