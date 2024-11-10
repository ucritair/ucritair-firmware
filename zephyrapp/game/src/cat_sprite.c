#include "cat_sprite.h"

#include <stdint.h>
#include <string.h>
#include "cat_core.h"

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
	FILE* file = fopen(path, "rb");
	if(file == NULL)
	{
		printf("Sprite %s not found! Loading null sprite\n", path);
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

	size_t row_size = png_get_rowbytes(png, info);
	png_bytepp rows = png_get_rows(png, info);
	uint16_t* pixels = CAT_malloc(sizeof(uint16_t) * width * height);
	for(int y = 0; y < height; y++)
	{
		uint8_t* r_row = rows[y];
		uint16_t* w_row = &pixels[y * width];
		for(int x = 0; x < width; x++)
		{
			uint8_t r_8 = r_row[x*4+0];
			uint8_t g_8 = r_row[x*4+1];
			uint8_t b_8 = r_row[x*4+2];
			uint8_t a_8 = r_row[x*4+3];
			float r_n = (float) r_8 / 255.0f;
			float g_n = (float) g_8 / 255.0f;
			float b_n = (float) b_8 / 255.0f;
			uint8_t r_5 = r_n * 31;
			uint8_t g_6 = g_n * 63;
			uint8_t b_5 = b_n * 31;
			uint16_t rgb_565 = (r_5 << 11) | (g_6 << 5) | b_5;
			w_row[x] = a_8 >= 255 ? rgb_565 : 0xdead;
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
		return -1;

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

CAT_sprite* CAT_sprite_get(int sprite_id)
{
	return &atlas.table[sprite_id];
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
		printf("CAT: E: Tried to draw invalid sprite %d\n", sprite_id);
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
		printf("CAT: E: Tried to draw invalid tile %d\n", sprite_id);
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
#if LCD_FRAMEBUFFER_SEGMENTS != 10
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

//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

CAT_draw_queue draw_queue;

void CAT_anim_toggle_loop(int sprite_id, bool toggle)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
		return;
	CAT_sprite* sprite = &atlas.table[sprite_id];
	sprite->loop = toggle;
}

void CAT_anim_toggle_reverse(int sprite_id, bool toggle)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
		return;
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
		return true;
	CAT_sprite* sprite = &atlas.table[sprite_id];
	if(sprite->reverse)
		return sprite->frame_idx == 0;
	else
		return sprite->frame_idx == sprite->frame_count-1;
}

void CAT_anim_reset(int sprite_id)
{
	if(sprite_id < 0 || sprite_id >= atlas.length)
		return;
	CAT_sprite* sprite = &atlas.table[sprite_id];
	if(sprite->reverse)
		sprite->frame_idx = sprite->frame_count-1;
	else
		sprite->frame_idx = 0;
}

void CAT_draw_queue_init()
{
	draw_queue.length = 0;
	draw_queue.anim_period = 0.15f;
	draw_queue.anim_timer = 0.0f;
}

void CAT_draw_queue_add(int sprite_id, int frame_idx, int layer, int x, int y, int mode)
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

	for(int i = draw_queue.length; i > insert_idx; i--)
	{
		draw_queue.jobs[i] = draw_queue.jobs[i-1];
	}
	draw_queue.jobs[insert_idx] = (CAT_draw_job) {sprite_id, frame_idx, layer, x, y, mode};
	draw_queue.length += 1;
}

void CAT_draw_queue_animate(int sprite_id, int layer, int x, int y, int mode)
{
	CAT_draw_queue_add(sprite_id, -1, layer, x, y, mode);
}

void CAT_draw_queue_submit(int cycle)
{
	if (cycle==0)
	{
		draw_queue.anim_timer += CAT_get_delta_time();
		if(draw_queue.anim_timer >= draw_queue.anim_period)
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
			draw_queue.anim_timer = 0.0f;
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
// ID DECLARATIONS

// TILESETS
int base_wall_sprite;
int base_floor_sprite;
int sky_wall_sprite;
int grass_floor_sprite;

// PET
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

int pet_crit_vig_sprite;
int pet_uncrit_vig_sprite;
int pet_crit_foc_sprite;
int pet_uncrit_foc_sprite;
int pet_crit_spi_sprite;
int pet_uncrit_spi_sprite;

int pet_vig_up_sprite;
int pet_foc_up_sprite;
int pet_spi_up_sprite;

int pet_eat_down_sprite;
int pet_eat_up_sprite;
int pet_chew_sprite;

int bubl_low_vig_sprite;
int bubl_low_foc_sprite;
int bubl_low_spi_sprite;
int bubl_react_good_sprite;
int bubl_react_bad_sprite;

// PROPS
int window_dawn_sprite;
int window_day_sprite;
int window_night_sprite;
int vending_sprite;

int table_sm_sprite;
int table_lg_sprite;
int chair_wood_sprite;
int stool_wood_sprite;
int stool_stone_sprite;
int stool_gold_sprite;

int coffeemaker_sprite;
int fan_sprite;
int solderpaste_sprite;
int purifier_sprite;
int uv_lamp_sprite;
int eth_purif_sprite;

int lantern_lit_sprite;
int lantern_unlit_sprite;
int bowl_stone_sprite;
int bowl_gold_sprite;
int vase_stone_sprite;
int vase_gold_sprite;

int poster_panda_sprite;
int poster_panda_zk_sprite;
int banner_sprite;
int pixel_sprite;

int effigy_blue_sprite;
int effigy_purple_sprite;
int effigy_sea_sprite;

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

int crystal_blue_sm_sprite;
int crystal_green_sm_sprite;
int crystal_purple_sm_sprite;
int crystal_blue_hrt_sprite;
int crystal_green_hrt_sprite;
int crystal_purple_hrt_sprite;
int crystal_blue_md_sprite;
int crystal_green_md_sprite;
int crystal_purple_md_sprite;
int crystal_blue_lg_sprite;
int crystal_green_lg_sprite;
int crystal_purple_lg_sprite;

// GRIMBA
int cigarette_sprite;
int sausage_sprite;
int padkrapow_sprite;
int coffee_sprite;
int book_static_sprite;
int book_study_sprite;
int toy_baseball_sprite;
int toy_duck_sprite;
int toy_basketball_sprite;
int mask_sprite;
int coin_sprite;

// WORLD UI
int cursor_sprite;
int cursor_add_sprite;
int cursor_flip_sprite;
int cursor_remove_sprite;
int tile_hl_sprite;
int tile_hl_add_sprite;
int tile_hl_flip_sprite;
int tile_mark_flip_sprite;
int tile_hl_rm_sprite;
int tile_mark_rm_sprite;
int touch_ring_sprite;

// CORE UI
int sbut_feed_sprite;
int sbut_study_sprite;
int sbut_play_sprite;
int sbut_deco_sprite;
int sbut_menu_sprite;
int sbut_hl_sprite;

int panel_sprite;
int glyph_sprite;
int strikethrough_sprite;
int icon_pointer_sprite;
int icon_enter_sprite;
int icon_exit_sprite;
int icon_equip_sprite;
int icon_item_sprite;
int icon_coin_sprite;

int fbut_a_sprite;
int fbut_b_sprite;
int fbut_n_sprite;
int fbut_e_sprite;
int fbut_s_sprite;
int fbut_w_sprite;
int fbut_start_sprite;
int fbut_select_sprite;

// STAT ICONS
int icon_vig_sprite;
int icon_foc_sprite;
int icon_spi_sprite;
int cell_vig_sprite;
int cell_foc_sprite;
int cell_spi_sprite;
int cell_empty_sprite;

// AQ ICONS
int icon_temp_sprite;
int icon_co2_sprite;
int icon_pm_sprite;
int icon_voc_sprite;
int icon_nox_sprite;

// EFFECTIVE ALTRUISM
int icon_mask_sprite;
int icon_pure_sprite;
int icon_uv_sprite;

// MACHINES
CAT_AM_state* pet_asm;
CAT_AM_state AS_idle;
CAT_AM_state AS_walk;
CAT_AM_state AS_adjust_in;
CAT_AM_state AS_walk_action;
CAT_AM_state AS_eat;
CAT_AM_state AS_study;
CAT_AM_state AS_play;
CAT_AM_state AS_adjust_out;
CAT_AM_state AS_vig_up;
CAT_AM_state AS_foc_up;
CAT_AM_state AS_spi_up;

CAT_AM_state* bubble_asm;
CAT_AM_state AS_react;

#ifndef CAT_BAKED_ASSETS
#ifdef LOUIS
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
	INIT_SPRITE(base_floor_sprite, "sprites/tile_basic.png", 3);
	INIT_SPRITE(sky_wall_sprite, "sprites/wall_sky.png", 8);
	INIT_SPRITE(grass_floor_sprite, "sprites/tile_grass.png", 9);

	// PET
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
	INIT_SPRITE(pet_idle_low_foc_sprite, "sprites/pet_unicorn_messy_a.png", 4);
	INIT_SPRITE(pet_walk_low_foc_sprite, "sprites/pet_unicorn_messy_walk_complex_a.png", 4);
	INIT_SPRITE(pet_idle_low_spi_sprite, "sprites/pet_unicorn_sad_idle_a.png", 4);
	INIT_SPRITE(pet_walk_low_spi_sprite, "sprites/pet_unicorn_sad_walk_a.png", 4);

	INIT_SPRITE(pet_vig_up_sprite, "sprites/pet_unicorn_stat_vigor_up_a.png", 13);
	INIT_SPRITE(pet_foc_up_sprite, "sprites/pet_unicorn_stat_focus_up_a.png", 13);
	INIT_SPRITE(pet_spi_up_sprite, "sprites/pet_unicorn_stat_spirit_up_a.png", 13);

	INIT_SPRITE(pet_crit_vig_sprite, "sprites/pet_unicorn_melt_a.png", 8);
	CAT_anim_toggle_loop(pet_crit_vig_sprite, false);
	COPY_SPRITE(pet_uncrit_vig_sprite, pet_crit_vig_sprite, false, true);
	INIT_SPRITE(pet_crit_foc_sprite, "sprites/pet_unicorn_tipped_a.png", 6);
	CAT_anim_toggle_loop(pet_crit_foc_sprite, false);
	COPY_SPRITE(pet_uncrit_foc_sprite, pet_crit_foc_sprite, false, true);
	INIT_SPRITE(pet_crit_spi_sprite, "sprites/pet_unicorn_block_a.png", 8);
	CAT_anim_toggle_loop(pet_crit_spi_sprite, false);
	COPY_SPRITE(pet_uncrit_spi_sprite, pet_crit_spi_sprite, false, true);

	INIT_SPRITE(pet_eat_down_sprite, "sprites/pet_unicorn_eat_lower_a.png", 7);
	COPY_SPRITE(pet_eat_up_sprite, pet_eat_down_sprite, false, true);
	INIT_SPRITE(pet_chew_sprite, "sprites/pet_unicorn_eat_chew_a.png", 2);

	INIT_SPRITE(bubl_low_vig_sprite, "sprites/bubl_low_vig.png", 3);
	INIT_SPRITE(bubl_low_foc_sprite, "sprites/bubl_low_foc.png", 3);
	INIT_SPRITE(bubl_low_spi_sprite, "sprites/bubl_low_spi.png", 3);
	INIT_SPRITE(bubl_react_good_sprite, "sprites/bubl_react_good.png", 5);
	INIT_SPRITE(bubl_react_bad_sprite, "sprites/bubl_react_bad.png", 3);

	// PROPS
	INIT_SPRITE(window_dawn_sprite, "sprites/prop_wall_window_dawn.png", 1);
	INIT_SPRITE(window_day_sprite, "sprites/prop_wall_window_day.png", 1);
	INIT_SPRITE(window_night_sprite, "sprites/prop_wall_window_dark.png", 1);
	INIT_SPRITE(vending_sprite, "sprites/interact_vending_items_a.png", 12);

	INIT_SPRITE(table_sm_sprite, "sprites/prop_table_md.png", 1);
	INIT_SPRITE(table_lg_sprite, "sprites/prop_table_xl_wood.png", 1);
	INIT_SPRITE(chair_wood_sprite, "sprites/prop_chair_wood.png", 4);
	INIT_SPRITE(stool_wood_sprite, "sprites/prop_stool_wood.png", 1);
	INIT_SPRITE(stool_stone_sprite, "sprites/prop_stool_stone.png", 1);
	INIT_SPRITE(stool_gold_sprite, "sprites/prop_stool_gold.png", 1);

	INIT_SPRITE(solderpaste_sprite, "sprites/prop_solderpaste.png", 1);
	INIT_SPRITE(coffeemaker_sprite, "sprites/prop_coffee_empty_a.png", 14);
	INIT_SPRITE(fan_sprite, "sprites/prop_fan_a.png", 3);
	INIT_SPRITE(purifier_sprite, "sprites/prop_purifier_a.png", 6);
	INIT_SPRITE(uv_lamp_sprite, "sprites/prop_uv_alt_a.png", 2);
	INIT_SPRITE(eth_purif_sprite, "sprites/prop_purifier_eth_a.png", 8);
	
	INIT_SPRITE(lantern_lit_sprite, "sprites/prop_lantern_lit_a.png", 2);
	INIT_SPRITE(lantern_unlit_sprite, "sprites/prop_lantern_unlit.png", 1);
	INIT_SPRITE(bowl_stone_sprite, "sprites/prop_bowl_stone_2.png", 1);
	INIT_SPRITE(bowl_gold_sprite, "sprites/prop_bowl_gold.png", 1);
	INIT_SPRITE(vase_stone_sprite, "sprites/prop_vase_sm_stone.png", 1);
	INIT_SPRITE(vase_gold_sprite, "sprites/prop_vase_sm_gold.png", 1);

	INIT_SPRITE(poster_panda_sprite, "sprites/prop_poster_md_panda.png", 1);
	INIT_SPRITE(poster_panda_zk_sprite, "sprites/prop_poster_md_zk.png", 1);
	INIT_SPRITE(banner_sprite, "sprites/prop_banner.png", 1);
	INIT_SPRITE(pixel_sprite, "sprites/prop_pixel.png", 1);

	INIT_SPRITE(effigy_blue_sprite, "sprites/prop_effigy_blue_a.png", 4);
	INIT_SPRITE(effigy_purple_sprite, "sprites/prop_effigy_purple_a.png", 4);
	INIT_SPRITE(effigy_sea_sprite, "sprites/prop_effigy_sea_a.png", 4);

	INIT_SPRITE(succulent_sprite, "sprites/prop_plant_md_stem.png", 1);
	INIT_SPRITE(bush_plain_sprite, "sprites/prop_bush_lg_empty.png", 1);
	INIT_SPRITE(bush_daisy_sprite, "sprites/prop_bush_lg_daisy.png", 1);
	INIT_SPRITE(bush_lilac_sprite, "sprites/prop_bush_lg_lilac.png", 1);
	INIT_SPRITE(plant_green_sprite, "sprites/prop_plant_sapling_green.png", 1);
	INIT_SPRITE(plant_maroon_sprite, "sprites/prop_plant_sapling_maroon.png", 1);
	INIT_SPRITE(plant_purple_sprite, "sprites/prop_plant_sapling_purple.png", 1);
	INIT_SPRITE(plant_yellow_sprite, "sprites/prop_plant_sapling_yellow.png", 1);
	INIT_SPRITE(flower_vig_sprite, "sprites/interact_stat_plant_vigor.png", 6);
	INIT_SPRITE(flower_foc_sprite, "sprites/interact_stat_plant_focus.png", 6);

	INIT_SPRITE(crystal_blue_sm_sprite, "sprites/prop_crystal_sm_shard_blue.png", 1);
	INIT_SPRITE(crystal_green_sm_sprite, "sprites/prop_crystal_sm_shard_green.png", 1);
	INIT_SPRITE(crystal_purple_sm_sprite, "sprites/prop_crystal_sm_shard_purple.png", 1);
	INIT_SPRITE(crystal_blue_hrt_sprite, "sprites/prop_crystal_sm_heart_blue.png", 1);
	INIT_SPRITE(crystal_green_hrt_sprite, "sprites/prop_crystal_sm_heart_green.png", 1);
	INIT_SPRITE(crystal_purple_hrt_sprite, "sprites/prop_crystal_sm_heart_purple.png", 1);
	INIT_SPRITE(crystal_blue_md_sprite, "sprites/prop_crystal_md_shard_blue.png", 1);
	INIT_SPRITE(crystal_green_md_sprite, "sprites/prop_crystal_md_shard_green.png", 1);
	INIT_SPRITE(crystal_purple_md_sprite, "sprites/prop_crystal_md_shard_purple.png", 1);
	INIT_SPRITE(crystal_blue_lg_sprite, "sprites/prop_crystal_lg_shard_blue.png", 1);
	INIT_SPRITE(crystal_green_lg_sprite, "sprites/prop_crystal_lg_shard_green.png", 1);
	INIT_SPRITE(crystal_purple_lg_sprite, "sprites/prop_crystal_lg_shard_purple.png", 1);

	// GRIMBA
	INIT_SPRITE(cigarette_sprite, "sprites/prop_cigarette.png", 1);
	INIT_SPRITE(sausage_sprite, "sprites/food_sausage_sm.png", 1);
	INIT_SPRITE(padkrapow_sprite, "sprites/food_padkrakow_sm.png", 1);
	INIT_SPRITE(coffee_sprite, "sprites/food_coffee_sm.png", 1);
	INIT_SPRITE(book_static_sprite, "sprites/read_book_upright.png", 1);
	INIT_SPRITE(book_study_sprite, "sprites/read_book_turn_a.png", 8);
	INIT_SPRITE(toy_baseball_sprite, "sprites/toy_baseball.png", 1);
	INIT_SPRITE(toy_duck_sprite, "sprites/toy_sm_duck.png", 1);
	INIT_SPRITE(toy_basketball_sprite, "sprites/toy_sm_basketball.png", 1);
	INIT_SPRITE(mask_sprite, "sprites/mask.png", 1);
	INIT_SPRITE(coin_sprite, "sprites/coin_a.png", 4);

	// WORLD UI
	INIT_SPRITE(cursor_sprite, "sprites/cursor_room_ornate.png", 1);
	INIT_SPRITE(cursor_add_sprite, "sprites/cursor_add.png", 1);
	INIT_SPRITE(cursor_flip_sprite, "sprites/cursor_flip.png", 1);
	INIT_SPRITE(cursor_remove_sprite, "sprites/cursor_remove.png", 1);
	INIT_SPRITE(tile_hl_sprite, "sprites/tile_hl.png", 1);
	INIT_SPRITE(tile_hl_add_sprite, "sprites/tile_hl_add.png", 1);
	INIT_SPRITE(tile_hl_flip_sprite, "sprites/tile_hl_flip.png", 1);
	INIT_SPRITE(tile_mark_flip_sprite, "sprites/tile_mark_flip.png", 1);
	INIT_SPRITE(tile_hl_rm_sprite, "sprites/tile_hl_rm.png", 1);
	INIT_SPRITE(tile_mark_rm_sprite, "sprites/tile_mark_rm.png", 1);
	INIT_SPRITE(touch_ring_sprite, "sprites/touch_ring_sprite.png", 1);

	// CORE UI
	INIT_SPRITE(sbut_feed_sprite, "sprites/Stat_Refill_Vigor_Button.png", 2);
	INIT_SPRITE(sbut_study_sprite, "sprites/Stat_Refill_Focus_Button.png", 2);
	INIT_SPRITE(sbut_play_sprite, "sprites/Stat_Refill_Spirit_Button.png", 2);
	INIT_SPRITE(sbut_deco_sprite, "sprites/sbut_deco.png", 2);
	INIT_SPRITE(sbut_menu_sprite, "sprites/sbut_menu.png", 2);
	INIT_SPRITE(sbut_hl_sprite, "sprites/sbut_hl_sprite.png", 1);
	
	INIT_SPRITE(panel_sprite, "sprites/panel_tiles.png", 10);
	INIT_SPRITE(glyph_sprite, "sprites/glyphs.png", 91);
	INIT_SPRITE(strikethrough_sprite, "sprites/strikethrough.png", 1);
	INIT_SPRITE(icon_pointer_sprite, "sprites/icon_pointer.png", 1);
	INIT_SPRITE(icon_enter_sprite, "sprites/icon_enter.png", 1);
	INIT_SPRITE(icon_exit_sprite, "sprites/icon_exit.png", 1);
	INIT_SPRITE(icon_equip_sprite, "sprites/icon_equip.png", 2);
	INIT_SPRITE(icon_item_sprite, "sprites/icon_item.png", 4);
	INIT_SPRITE(icon_coin_sprite, "sprites/icon_coin.png", 1);

	INIT_SPRITE(fbut_a_sprite, "sprites/A Button_Both.png", 2);
	INIT_SPRITE(fbut_b_sprite, "sprites/B Button_Both.png", 2);
	INIT_SPRITE(fbut_n_sprite, "sprites/Up_Arrow_Both.png", 2);
	INIT_SPRITE(fbut_e_sprite, "sprites/Right Arrow_Both.png", 2);
	INIT_SPRITE(fbut_s_sprite, "sprites/Down Arrow_Both.png", 2);
	INIT_SPRITE(fbut_w_sprite, "sprites/Left Arrow_Both.png", 2);
	INIT_SPRITE(fbut_start_sprite, "sprites/Start Button_Both.png", 2);
	INIT_SPRITE(fbut_select_sprite, "sprites/Select Button_Both.png", 2);
	
	INIT_SPRITE(icon_vig_sprite, "sprites/STAT_VIGOR2424.png", 1);
	INIT_SPRITE(icon_foc_sprite, "sprites/STAT_FOCUS2424.png", 1);
	INIT_SPRITE(icon_spi_sprite, "sprites/STAT_SPIRIT2424.png", 1);
	INIT_SPRITE(cell_vig_sprite, "sprites/cell_vig.png", 1);
	INIT_SPRITE(cell_foc_sprite, "sprites/cell_foc.png", 1);
	INIT_SPRITE(cell_spi_sprite, "sprites/cell_spi.png", 1);
	INIT_SPRITE(cell_empty_sprite, "sprites/cell_empty.png", 1);

	INIT_SPRITE(icon_temp_sprite, "sprites/icon_temp.png", 3);
	INIT_SPRITE(icon_co2_sprite, "sprites/icon_co2.png", 3);
	INIT_SPRITE(icon_pm_sprite, "sprites/icon_pm.png", 3);
	INIT_SPRITE(icon_voc_sprite, "sprites/icon_voc.png", 3);
	INIT_SPRITE(icon_nox_sprite, "sprites/icon_nox.png", 3);

	INIT_SPRITE(icon_mask_sprite, "sprites/icon_mask.png", 1);
	INIT_SPRITE(icon_pure_sprite, "sprites/icon_pure.png", 1);
	INIT_SPRITE(icon_uv_sprite, "sprites/icon_uv.png", 1);

	// MACHINES
	CAT_AM_init(&AS_idle, -1, pet_idle_sprite, -1);
	CAT_AM_init(&AS_walk, -1, pet_walk_sprite, -1);
	CAT_AM_init(&AS_adjust_in, -1, -1, pet_idle_sprite);
	CAT_AM_init(&AS_walk_action, -1, pet_walk_sprite, -1);
	CAT_AM_init(&AS_eat, pet_eat_down_sprite, pet_chew_sprite, pet_eat_up_sprite);
	CAT_AM_init(&AS_study, pet_eat_down_sprite, pet_chew_sprite, pet_eat_up_sprite);
	CAT_AM_init(&AS_play, pet_eat_down_sprite, pet_chew_sprite, pet_eat_up_sprite);
	CAT_AM_init(&AS_adjust_out, -1, -1, pet_idle_sprite);
	CAT_AM_init(&AS_vig_up, -1, -1, pet_vig_up_sprite);
	CAT_AM_init(&AS_foc_up, -1, -1, pet_foc_up_sprite);
	CAT_AM_init(&AS_spi_up, -1, -1, pet_spi_up_sprite);

	CAT_AM_init(&AS_react, -1, bubl_react_good_sprite, -1);
}
