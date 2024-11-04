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
#else
extern const uint16_t** image_data_table[];
extern uint16_t rle_work_region[];
#include "lcd_driver.h"
#endif
#endif

#ifndef CAT_EMBEDDED
#define LCD_FRAMEBUFFER_H LCD_SCREEN_H
#endif

void CAT_atlas_init()
{
	atlas.length = 0;
}

#ifndef CAT_BAKED_ASSETS

#include "png.h"

int CAT_sprite_init(const char* path, int frame_count)
{
	FILE* file = fopen(path, "rb");
	if(file == NULL)
	{
		printf("Sprite %s not found! Loading null sprite\n", path);
		file = fopen("sprites/none_32x32.png", "rb");
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
	sprite.width = width;
	sprite.height = height / frame_count;
	sprite.frame_count = frame_count;
	sprite.frame_idx = 0;
	sprite.loop = true;
	sprite.needs_update = false;

	int sprite_id = atlas.length;
	atlas.table[sprite_id] = sprite;
	atlas.length += 1;
	return sprite_id;
}

void CAT_atlas_cleanup()
{
	for(int i = 0; i < atlas.length; i++)
	{
		CAT_free(atlas.table[i].pixels);
	}
}

#endif

#ifndef CAT_DESKTOP
extern uint16_t lcd_framebuffer[];
#endif

CAT_spriter spriter;

void CAT_spriter_init()
{
#ifdef CAT_DESKTOP
	spriter.framebuffer = CAT_malloc(sizeof(uint16_t) * LCD_SCREEN_W * LCD_SCREEN_H);
#else
	spriter.framebuffer = lcd_framebuffer;
#endif
	spriter.mode = CAT_DRAW_MODE_DEFAULT;
}

#ifdef CAT_BAKED_ASSETS

struct {
	const uint16_t* ptr;
	int width;
	int rle_count;
	uint16_t rle_word;
} rle_decode_state;

void init_rle_decode(const uint16_t* data, int width)
{
	rle_decode_state.ptr = data;
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

		if (word == 0xbeef)
		{
			rle_decode_state.rle_word = RLE_NEXT();
			rle_decode_state.rle_count = RLE_NEXT();

			// printf("rep %04x cnt %04x\n", repeated, repeat_count);
		}
		else
		{
			rle_work_region[unpacked++] = word;
			// printf("direct\n");
		}
	}
}

#endif


void CAT_draw_sprite(int sprite_id, int frame_idx, int x, int y)
{
	CAT_sprite sprite = atlas.table[sprite_id];
	int w = sprite.width;
	int h = sprite.height;

#ifdef CAT_BAKED_ASSETS
	init_rle_decode(image_data_table[sprite_id][frame_idx], sprite.width);
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
#endif
	
	for(int dy = 0; dy < h; dy++)
	{
		int y_w = y+dy;

#ifdef CAT_BAKED_ASSETS
		unpack_rle_row();
#endif
		
		if(y_w < 0 || y_w >= LCD_FRAMEBUFFER_H)
			continue;

		for(int dx = 0; dx < w; dx++)
		{
			int x_w = x+dx;
			if(x_w < 0 || x_w >= LCD_SCREEN_W)
				continue;
			
			int row_offset = dy * w;

			int col_offset;
			if ((spriter.mode & CAT_DRAW_MODE_REFLECT_X) > 0)
				col_offset = w - dx - 1;
			else
				col_offset = dx;

			int w_idx = y_w * LCD_SCREEN_W + x_w;
#ifdef CAT_BAKED_ASSETS
			uint16_t px = rle_work_region[col_offset];
#else
			uint16_t px = frame[row_offset + col_offset];
#endif

			if(px != 0xdead)
				spriter.framebuffer[w_idx] = px;
		}
	}
}

void CAT_draw_tiles(int sprite_id, int frame_idx, int y_t, int h_t)
{
	CAT_sprite sprite = atlas.table[sprite_id];

#ifndef CAT_BAKED_ASSETS
	const uint16_t* frame = &sprite.pixels[frame_idx * CAT_TILE_SIZE * CAT_TILE_SIZE];
#endif

	int y_start = y_t * CAT_TILE_SIZE;
	int y_end = (y_t+h_t) * CAT_TILE_SIZE;

#ifdef CAT_EMBEDDED
	y_start -= framebuffer_offset_h;
	y_end -= framebuffer_offset_h;
#endif

	for(int y_w = y_start; y_w < y_end; y_w += CAT_TILE_SIZE)
	{
		if(y_w < 0 || y_w >= LCD_FRAMEBUFFER_H)
			continue;

		for(int x_w = 0; x_w < LCD_SCREEN_W; x_w += CAT_TILE_SIZE)
		{

#ifdef CAT_BAKED_ASSETS
			init_rle_decode(image_data_table[sprite_id][frame_idx], sprite.width);
#endif

			for(int dy = 0; dy < CAT_TILE_SIZE; dy++)
			{

#ifdef CAT_BAKED_ASSETS
				unpack_rle_row();
#endif

				for(int dx = 0; dx < CAT_TILE_SIZE; dx++)
				{
					int w_idx = (y_w+dy) * LCD_SCREEN_W + (x_w+dx);
					int row_offset = dy * CAT_TILE_SIZE;
					
#ifndef CAT_BAKED_ASSETS
					uint16_t px = frame[row_offset + dx];
#else
					uint16_t px = rle_work_region[dx];
#endif

					spriter.framebuffer[w_idx] = px;
				}
			}
		}
	}
}

void CAT_spriter_cleanup()
{
	CAT_free(spriter.framebuffer);
}

//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

CAT_draw_queue draw_queue;

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

void CAT_draw_queue_add_anim(int sprite_id, int layer, int x, int y, int mode)
{
	CAT_draw_queue_add(sprite_id, -1, layer, x, y, mode);
}

bool CAT_anim_finished(int sprite_id)
{
	return atlas.table[sprite_id].frame_idx == atlas.table[sprite_id].frame_count-1;
}

void CAT_anim_reset(int sprite_id)
{
	atlas.table[sprite_id].frame_idx = 0;
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
				
				if(sprite->frame_idx < sprite->frame_count-1)
				{
					sprite->frame_idx += 1;
				}
				else if(sprite->loop)
				{
					sprite->frame_idx = 0;
				}
				sprite->needs_update = false;
			}
			draw_queue.anim_timer = 0.0f;
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

		if (cycle==0)
		{
			sprite->needs_update = true;
		}
	}
	draw_queue.length = 0;
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

int pet_idle_low_vig_sprite;
int pet_walk_low_vig_sprite;
int pet_idle_low_foc_sprite;
int pet_walk_low_foc_sprite;
int pet_idle_low_spi_sprite;
int pet_walk_low_spi_sprite;

int pet_crit_vig_sprite;
int pet_crit_foc_sprite;
int pet_crit_spi_sprite;

int pet_vig_up_sprite;
int pet_foc_up_sprite;
int pet_spi_up_sprite;

int pet_eat_sprite;
int pet_chew_sprite;
int pet_die_sprite;

// PROPS
int window_dawn_sprite;
int window_day_sprite;
int window_night_sprite;
int vending_sprite;
int solderpaste_sprite;
int flower_empty_sprite;
int flower_vig_sprite;
int flower_foc_sprite;
int flower_spi_sprite;
int table_sm_sprite;
int table_lg_sprite;
int chair_wood_sprite;
int stool_wood_sprite;
int stool_stone_sprite;
int stool_gold_sprite;
int coffee_sprite;
int fan_sprite;
int lantern_lit_sprite;
int lantern_unlit_sprite;
int bowl_stone_sprite;
int bowl_gold_sprite;
int vase_stone_sprite;
int vase_gold_sprite;
int bush_plain_sprite;
int bush_daisy_sprite;
int bush_lilac_sprite;
int plant_green_sprite;
int plant_maroon_sprite;
int plant_purple_sprite;
int plant_yellow_sprite;
int succulent_sprite;
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
int seed_vig_sprite;
int seed_foc_sprite;
int seed_spi_sprite;
int cigarette_sprite;

// WORLD UI
int cursor_sprite;
int cursor_deco_sprite;
int tile_hl_neg;
int tile_hl_inner;

// SCREEN UI
int fbut_a_sprite;
int fbut_b_sprite;
int fbut_n_sprite;
int fbut_e_sprite;
int fbut_s_sprite;
int fbut_w_sprite;
int fbut_start_sprite;
int fbut_select_sprite;
int sbut_feed_sprite;
int sbut_study_sprite;
int sbut_play_sprite;
int sbut_deco_sprite;
int sbut_menu_sprite;
int sbut_hl_sprite;
int icon_vig_sprite;
int icon_foc_sprite;
int icon_spi_sprite;
int icon_food_sprite;
int icon_prop_sprite;
int icon_key_sprite;
int panel_sprite;
int glyph_sprite;
int icon_pointer_sprite;
int icon_enter_sprite;
int icon_exit_sprite;
int cell_vig_sprite;
int cell_foc_sprite;
int cell_spi_sprite;
int cell_empty_sprite;
int crisis_co2_sprite;
int crisis_nox_sprite;
int crisis_vol_sprite;
int crisis_hot_sprite;
int crisis_cold_sprite;

#ifndef CAT_BAKED_ASSETS
#define INIT_SPRITE(name, path, frames) name = CAT_sprite_init(path, frames);\
										printf("BAKE: (%d, \"%s\", \"%s\", %d, %d, %d)\n", name, #name, path, frames, atlas.table[name].width, atlas.table[name].height);
#else
int sprite_count = 0;
#define INIT_SPRITE(name, path, frames) name = sprite_count++;
#endif

void CAT_sprite_mass_define()
{
	// TILESETS
	INIT_SPRITE(base_wall_sprite, "sprites/wall_basic.png", 3);
	INIT_SPRITE(base_floor_sprite, "sprites/tile_basic.png", 3);
	INIT_SPRITE(sky_wall_sprite, "sprites/wall_sky.png", 8);
	INIT_SPRITE(grass_floor_sprite, "sprites/tile_grass.png", 9);

	// PET
	INIT_SPRITE(pet_idle_sprite, "sprites/pet_unicorn_idle_complex_a.png", 4);
	INIT_SPRITE(pet_walk_sprite, "sprites/pet_unicorn_default_walk_complex_a.png", 4);

	INIT_SPRITE(pet_idle_high_vig_sprite, "sprites/pet_unicorn_idle_complex_a.png", 4);
	INIT_SPRITE(pet_walk_high_vig_sprite, "sprites/pet_unicorn_default_walk_complex_a.png", 4);
	INIT_SPRITE(pet_idle_high_foc_sprite, "sprites/pet_unicorn_idle_complex_a.png", 4);
	INIT_SPRITE(pet_walk_high_foc_sprite, "sprites/pet_unicorn_default_walk_complex_a.png", 4);
	INIT_SPRITE(pet_idle_high_spi_sprite, "sprites/pet_unicorn_idle_complex_a.png", 4);
	INIT_SPRITE(pet_walk_high_spi_sprite, "sprites/pet_unicorn_default_walk_complex_a.png", 4);

	INIT_SPRITE(pet_idle_low_vig_sprite, "sprites/pet_unicorn_idle_complex_a.png", 4);
	INIT_SPRITE(pet_walk_low_vig_sprite, "sprites/pet_unicorn_default_walk_complex_a.png", 4);
	INIT_SPRITE(pet_idle_low_foc_sprite, "sprites/pet_unicorn_idle_complex_a.png", 4);
	INIT_SPRITE(pet_walk_low_foc_sprite, "sprites/pet_unicorn_default_walk_complex_a.png", 4);
	INIT_SPRITE(pet_idle_low_spi_sprite, "sprites/pet_unicorn_idle_complex_a.png", 4);
	INIT_SPRITE(pet_walk_low_spi_sprite, "sprites/pet_unicorn_default_walk_complex_a.png", 4);

	INIT_SPRITE(pet_vig_up_sprite, "sprites/pet_unicorn_stat_vigor_up_a.png", 13);
	INIT_SPRITE(pet_foc_up_sprite, "sprites/pet_unicorn_stat_focus_up_a.png", 13);
	INIT_SPRITE(pet_spi_up_sprite, "sprites/pet_unicorn_stat_spirit_up_a.png", 13);

	INIT_SPRITE(pet_eat_sprite, "sprites/pet_unicorn_eat_lower_a.png", 7);
	INIT_SPRITE(pet_chew_sprite, "sprites/pet_unicorn_eat_chew_a.png", 2);

	// PROPS
	INIT_SPRITE(window_dawn_sprite, "sprites/prop_wall_window_dawn.png", 1);
	INIT_SPRITE(window_day_sprite, "sprites/prop_wall_window_day.png", 1);
	INIT_SPRITE(window_night_sprite, "sprites/prop_wall_window_dark.png", 1);
	INIT_SPRITE(vending_sprite, "sprites/interact_vending_items_a.png", 12);
	INIT_SPRITE(solderpaste_sprite, "sprites/prop_solderpaste.png", 1);
	INIT_SPRITE(flower_empty_sprite, "sprites/interact_stat_plant_empty.png", 1);
	INIT_SPRITE(flower_vig_sprite, "sprites/interact_stat_plant_vigor.png", 6);
	INIT_SPRITE(flower_foc_sprite, "sprites/interact_stat_plant_focus.png", 6);
	INIT_SPRITE(flower_spi_sprite, "sprites/interact_stat_plant_spirit.png", 6);
	INIT_SPRITE(table_sm_sprite, "sprites/prop_table_md.png", 1);
	INIT_SPRITE(table_lg_sprite, "sprites/prop_table_xl_wood.png", 1);
	INIT_SPRITE(chair_wood_sprite, "sprites/prop_chair_wood.png", 4);
	INIT_SPRITE(stool_wood_sprite, "sprites/prop_stool_wood.png", 1);
	INIT_SPRITE(stool_stone_sprite, "sprites/prop_stool_stone.png", 1);
	INIT_SPRITE(stool_gold_sprite, "sprites/prop_stool_gold.png", 1);
	INIT_SPRITE(coffee_sprite, "sprites/prop_coffee_empty_a.png", 14);
	INIT_SPRITE(fan_sprite, "sprites/prop_fan_a.png", 3);
	INIT_SPRITE(lantern_lit_sprite, "sprites/prop_lantern_lit_a.png", 2);
	INIT_SPRITE(lantern_unlit_sprite, "sprites/prop_lantern_unlit.png", 1);
	INIT_SPRITE(bowl_stone_sprite, "sprites/prop_bowl_stone_2.png", 1);
	INIT_SPRITE(bowl_gold_sprite, "sprites/prop_bowl_gold.png", 1);
	INIT_SPRITE(vase_stone_sprite, "sprites/prop_vase_sm_stone.png", 1);
	INIT_SPRITE(vase_gold_sprite, "sprites/prop_vase_sm_gold.png", 1);
	INIT_SPRITE(bush_plain_sprite, "sprites/prop_bush_lg_empty.png", 1);
	INIT_SPRITE(bush_daisy_sprite, "sprites/prop_bush_lg_daisy.png", 1);
	INIT_SPRITE(bush_lilac_sprite, "sprites/prop_bush_lg_lilac.png", 1);
	INIT_SPRITE(plant_green_sprite, "sprites/prop_plant_sapling_green.png", 1);
	INIT_SPRITE(plant_maroon_sprite, "sprites/prop_plant_sapling_maroon.png", 1);
	INIT_SPRITE(plant_purple_sprite, "sprites/prop_plant_sapling_purple.png", 1);
	INIT_SPRITE(plant_yellow_sprite, "sprites/prop_plant_sapling_yellow.png", 1);
	INIT_SPRITE(succulent_sprite, "sprites/prop_plant_md_stem.png", 1);
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

	// WORLD UI
	INIT_SPRITE(cursor_sprite, "sprites/cursor_room_ornate.png", 1);
	INIT_SPRITE(cursor_deco_sprite, "sprites/cursor_prop_plus_bordered_a.png", 4);
	INIT_SPRITE(tile_hl_neg, "sprites/tile_hl_neg.png", 1);
	INIT_SPRITE(tile_hl_inner, "sprites/tile_hl_inner.png", 1);

	// SCREEN UI
	INIT_SPRITE(fbut_a_sprite, "sprites/A Button_Both.png", 2);
	INIT_SPRITE(fbut_b_sprite, "sprites/B Button_Both.png", 2);
	INIT_SPRITE(fbut_n_sprite, "sprites/Up_Arrow_Both.png", 2);
	INIT_SPRITE(fbut_e_sprite, "sprites/Right Arrow_Both.png", 2);
	INIT_SPRITE(fbut_s_sprite, "sprites/Down Arrow_Both.png", 2);
	INIT_SPRITE(fbut_w_sprite, "sprites/Left Arrow_Both.png", 2);
	INIT_SPRITE(fbut_start_sprite, "sprites/Start Button_Both.png", 2);
	INIT_SPRITE(fbut_select_sprite, "sprites/Select Button_Both.png", 2);
	INIT_SPRITE(sbut_feed_sprite, "sprites/Stat_Refill_Vigor_Button.png", 2);
	INIT_SPRITE(sbut_study_sprite, "sprites/Stat_Refill_Focus_Button.png", 2);
	INIT_SPRITE(sbut_play_sprite, "sprites/Stat_Refill_Spirit_Button.png", 2);
	INIT_SPRITE(sbut_deco_sprite, "sprites/Deco_Button.png", 1);
	INIT_SPRITE(sbut_menu_sprite, "sprites/Menu_Button.png", 1);
	INIT_SPRITE(sbut_hl_sprite, "sprites/sbut_hl_sprite.png", 1);
	INIT_SPRITE(icon_vig_sprite, "sprites/STAT_VIGOR2424.png", 1);
	INIT_SPRITE(icon_foc_sprite, "sprites/STAT_FOCUS2424.png", 1);
	INIT_SPRITE(icon_spi_sprite, "sprites/STAT_SPIRIT2424.png", 1);
	INIT_SPRITE(icon_food_sprite, "sprites/icon_food.png", 1);
	INIT_SPRITE(icon_prop_sprite, "sprites/icon_prop.png", 1);
	INIT_SPRITE(icon_key_sprite, "sprites/icon_key.png", 1);
	INIT_SPRITE(panel_sprite, "sprites/panel_tiles.png", 9);
	INIT_SPRITE(glyph_sprite, "sprites/glyphs.png", 91);
	INIT_SPRITE(icon_pointer_sprite, "sprites/icon_pointer.png", 1);
	INIT_SPRITE(icon_enter_sprite, "sprites/icon_enter.png", 1);
	INIT_SPRITE(icon_exit_sprite, "sprites/icon_exit.png", 1);
	INIT_SPRITE(cell_vig_sprite, "sprites/cell_vig.png", 1);
	INIT_SPRITE(cell_foc_sprite, "sprites/cell_foc.png", 1);
	INIT_SPRITE(cell_spi_sprite, "sprites/cell_spi.png", 1);
	INIT_SPRITE(cell_empty_sprite, "sprites/cell_empty.png", 1);
	INIT_SPRITE(crisis_co2_sprite, "sprites/crisis_test_1.png", 1);
	INIT_SPRITE(crisis_nox_sprite, "sprites/crisis_test_2.png", 1);
	INIT_SPRITE(crisis_vol_sprite, "sprites/crisis_test_3.png", 1);
	INIT_SPRITE(crisis_hot_sprite, "sprites/crisis_test_temp_high.png", 1);
	INIT_SPRITE(crisis_cold_sprite, "sprites/crisis_test_temp_low.png", 1);
}
