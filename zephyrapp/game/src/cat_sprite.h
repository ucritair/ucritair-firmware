#ifndef CAT_SPRITE_H
#define CAT_SPRITE_H

#include <stdint.h>
#include <stdbool.h>

#include "cat_core.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_ATLAS_MAX_LENGTH 512
#define CAT_DRAW_QUEUE_MAX_LENGTH 128
#define CAT_ANIM_MAX_LENGTH 16
#define CAT_ANIM_TABLE_MAX_LENGTH 128
#define CAT_ANIM_QUEUE_MAX_LENGTH 128

#define CAT_TILE_SIZE 16


//////////////////////////////////////////////////////////////////////////
// ATLAS AND SPRITER

#ifdef CAT_BAKED_ASSETS
typedef struct {
	const uint16_t* color_table;
	const uint8_t** frames;
} CAT_baked_sprite;
#endif

typedef struct CAT_sprite
{
#ifndef CAT_BAKED_ASSETS
	uint16_t* pixels;
#endif

	int width;
	int height;
	int frame_count;

	int frame_idx;
	bool loop;
	bool reverse;
	bool needs_update;
} CAT_sprite;

typedef struct CAT_atlas
{
	CAT_sprite table[CAT_ATLAS_MAX_LENGTH];
	int length;
} CAT_atlas;
extern CAT_atlas atlas;

void CAT_atlas_init();
int CAT_sprite_init(const char* path, int frame_count);
int CAT_sprite_copy(int sprite_id, bool loop, bool reverse);
void CAT_atlas_cleanup();

typedef enum CAT_draw_mode
{
	CAT_DRAW_MODE_DEFAULT = 0,
	CAT_DRAW_MODE_BOTTOM = 1,
	CAT_DRAW_MODE_CENTER_X = 2,
	CAT_DRAW_MODE_CENTER_Y = 4,
	CAT_DRAW_MODE_REFLECT_X = 8
} CAT_draw_mode;

typedef struct CAT_spriter
{
	uint16_t* framebuffer;
	int mode;
} CAT_spriter;
extern CAT_spriter spriter;

void CAT_spriter_init();
void CAT_draw_sprite(int sprite_id, int frame_idx, int x, int y);
void CAT_draw_tiles(int sprite_id, int frame_idx, int y_t, int h_t);
void CAT_spriter_cleanup();


//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

typedef struct CAT_draw_job
{
	int sprite_id;
	int frame_idx;
	int layer;
	int x;
	int y;
	int mode;
} CAT_draw_job;

typedef struct CAT_draw_queue
{
	CAT_draw_job jobs[CAT_DRAW_QUEUE_MAX_LENGTH];
	int length;

	float anim_period;
	float anim_timer;
} CAT_draw_queue;
extern CAT_draw_queue draw_queue;

void CAT_anim_toggle_loop(int sprite_id, bool toggle);
void CAT_anim_toggle_reverse(int sprite_id, bool toggle);
bool CAT_anim_finished(int sprite_id);
void CAT_anim_reset(int sprite_id);

void CAT_draw_queue_init();
void CAT_draw_queue_add(int sprite_id, int frame_idx, int layer, int x, int y, int mode);
void CAT_draw_queue_animate(int sprite_id, int layer, int x, int y, int mode);
void CAT_draw_queue_submit(int cycle);


//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

// TILESETS
extern int base_wall_sprite;
extern int base_floor_sprite;
extern int sky_wall_sprite;
extern int grass_floor_sprite;


// PET
extern int pet_idle_sprite;
extern int pet_walk_sprite;

extern int pet_idle_high_vig_sprite;
extern int pet_walk_high_vig_sprite;
extern int pet_idle_high_foc_sprite;
extern int pet_walk_high_foc_sprite;
extern int pet_idle_high_spi_sprite;
extern int pet_walk_high_spi_sprite;
extern int pet_wings_out_sprite;
extern int pet_wings_in_sprite;

extern int pet_idle_low_vig_sprite;
extern int pet_walk_low_vig_sprite;
extern int pet_idle_low_foc_sprite;
extern int pet_walk_low_foc_sprite;
extern int pet_idle_low_spi_sprite;
extern int pet_walk_low_spi_sprite;

extern int pet_crit_vig_sprite;
extern int pet_crit_foc_sprite;
extern int pet_crit_spi_sprite;

extern int pet_vig_up_sprite;
extern int pet_foc_up_sprite;
extern int pet_spi_up_sprite;

extern int pet_eat_down_sprite;
extern int pet_eat_up_sprite;
extern int pet_chew_sprite;

extern int bubl_low_vig_sprite;
extern int bubl_low_foc_sprite;
extern int bubl_low_spi_sprite;
extern int bubl_react_good_sprite;
extern int bubl_react_bad_sprite;


// PROPS
extern int window_dawn_sprite;
extern int window_day_sprite;
extern int window_night_sprite;
extern int vending_sprite;

extern int table_sm_sprite;
extern int table_lg_sprite;
extern int chair_wood_sprite;
extern int stool_wood_sprite;
extern int stool_stone_sprite;
extern int stool_gold_sprite;

extern int coffeemaker_sprite;
extern int fan_sprite;
extern int solderpaste_sprite;
extern int purifier_sprite;
extern int uv_lamp_sprite;

extern int lantern_lit_sprite;
extern int lantern_unlit_sprite;
extern int bowl_stone_sprite;
extern int bowl_gold_sprite;
extern int vase_stone_sprite;
extern int vase_gold_sprite;

extern int succulent_sprite;
extern int bush_plain_sprite;
extern int bush_daisy_sprite;
extern int bush_lilac_sprite;
extern int plant_green_sprite;
extern int plant_maroon_sprite;
extern int plant_purple_sprite;
extern int plant_yellow_sprite;
extern int flower_vig_sprite;
extern int flower_foc_sprite;

extern int crystal_blue_sm_sprite;
extern int crystal_green_sm_sprite;
extern int crystal_purple_sm_sprite;
extern int crystal_blue_hrt_sprite;
extern int crystal_green_hrt_sprite;
extern int crystal_purple_hrt_sprite;
extern int crystal_blue_md_sprite;
extern int crystal_green_md_sprite;
extern int crystal_purple_md_sprite;
extern int crystal_blue_lg_sprite;
extern int crystal_green_lg_sprite;
extern int crystal_purple_lg_sprite;


// FOOD
extern int cigarette_sprite;
extern int sausage_sprite;
extern int padkrapow_sprite;
extern int coffee_sprite;


// WORLD UI
extern int cursor_sprite;
extern int cursor_add_sprite;
extern int cursor_flip_sprite;
extern int cursor_remove_sprite;
extern int tile_hl_sprite;
extern int tile_hl_add_sprite;
extern int tile_hl_flip_sprite;
extern int tile_mark_flip_sprite;
extern int tile_hl_rm_sprite;
extern int tile_mark_rm_sprite;

// CORE UI
extern int sbut_feed_sprite;
extern int sbut_study_sprite;
extern int sbut_play_sprite;
extern int sbut_deco_sprite;
extern int sbut_menu_sprite;
extern int sbut_hl_sprite;

extern int panel_sprite;
extern int glyph_sprite;
extern int strikethrough_sprite;
extern int icon_pointer_sprite;
extern int icon_enter_sprite;
extern int icon_exit_sprite;

extern int fbut_a_sprite;
extern int fbut_b_sprite;
extern int fbut_n_sprite;
extern int fbut_e_sprite;
extern int fbut_s_sprite;
extern int fbut_w_sprite;
extern int fbut_start_sprite;
extern int fbut_select_sprite;

// MENU ICONS 
extern int icon_food_sprite;
extern int icon_prop_sprite;
extern int icon_key_sprite;

extern int icon_vig_sprite;
extern int icon_foc_sprite;
extern int icon_spi_sprite;
extern int cell_vig_sprite;
extern int cell_foc_sprite;
extern int cell_spi_sprite;
extern int cell_empty_sprite;

// AQ ICONS
extern int icon_temp_sprite[3];
extern int icon_co2_sprite[3];
extern int icon_pm_sprite[3];
extern int icon_voc_sprite[3];
extern int icon_nox_sprite[3];

void CAT_sprite_mass_define();

#endif
