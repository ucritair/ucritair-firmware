#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cat_core.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS AND MACROS

#define CAT_ATLAS_MAX_LENGTH 512
#define CAT_DRAW_QUEUE_MAX_LENGTH 128
#define CAT_ANIM_MAX_LENGTH 16
#define CAT_ANIM_TABLE_MAX_LENGTH 128
#define CAT_ANIM_QUEUE_MAX_LENGTH 128

#define CAT_TILE_SIZE 16

#define RGB8882565(r, g, b) ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3)


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
	bool duplicate;
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
CAT_sprite* CAT_sprite_get(int sprite_id);
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
#ifdef CAT_DESKTOP
	uint16_t* framebuffer;
#endif
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
} CAT_draw_queue;
extern CAT_draw_queue draw_queue;

void CAT_anim_toggle_loop(int sprite_id, bool toggle);
void CAT_anim_toggle_reverse(int sprite_id, bool toggle);
bool CAT_anim_finished(int sprite_id);
void CAT_anim_reset(int sprite_id);

void CAT_draw_queue_insert(int idx, int sprite_id, int frame_idx, int layer, int x, int y, int mode);
int CAT_draw_queue_add(int sprite_id, int frame_idx, int layer, int x, int y, int mode);
int CAT_draw_queue_animate(int sprite_id, int layer, int x, int y, int mode);
void CAT_draw_queue_submit(int cycle);


//////////////////////////////////////////////////////////////////////////
// ANIMATION MACHINE

typedef struct CAT_animachine_state
{
	enum {ENTER, TICK, EXIT, DONE} signal;

	int enter_anim_id;
	int tick_anim_id;
	int exit_anim_id;

	struct CAT_animachine_state* next;
} CAT_animachine_state;

void CAT_animachine_init(CAT_animachine_state* state, int enai, int tiai, int exai);
void CAT_animachine_transition(CAT_animachine_state** spp, CAT_animachine_state* next);
int CAT_animachine_tick(CAT_animachine_state** pp);
void CAT_animachine_kill(CAT_animachine_state** spp);

bool CAT_animachine_is_in(CAT_animachine_state** spp, CAT_animachine_state* state);
bool CAT_animachine_is_ticking(CAT_animachine_state** spp);


//////////////////////////////////////////////////////////////////////////
// THE BERRIER

void CAT_greenberry(int xi, int w, int yi, int h, float t);
void CAT_frameberry(uint16_t c);
void CAT_greyberry(int xi, int w, int yi, int h);
void CAT_roundberry(int xi, int yi, int r, uint16_t c);
void CAT_lineberry(int xi, int yi, int xf, int yf, uint16_t c);
void CAT_depthberry();
void CAT_triberry
(
	int xa, int ya, float za,
	int xb, int yb, float zb,
	int xc, int yc, float zc,
	uint16_t c
);
void CAT_fillberry(int xi, int yi, int w, int h, uint16_t c);
void CAT_strokeberry(int xi, int yi, int w, int h, uint16_t c);


//////////////////////////////////////////////////////////////////////////
// DECLARATIONS

// TILESETS
extern int base_wall_sprite;
extern int sky_wall_sprite;
extern int base_floor_sprite;
extern int grass_floor_sprite;
extern int ash_floor_sprite;

extern int glyph_sprite;
extern int strike_sprite;
extern int tab_sprite;

// ICONS
extern int icon_pointer_sprite;
extern int icon_a_sprite;
extern int icon_b_sprite;
extern int icon_n_sprite;
extern int icon_e_sprite;
extern int icon_s_sprite;
extern int icon_w_sprite;
extern int icon_start_sprite;
extern int icon_select_sprite;
extern int icon_enter_sprite;
extern int icon_exit_sprite;
extern int icon_plot_sprite;
extern int icon_equip_sprite;
extern int icon_input_sprite;

extern int icon_item_key_sprite;
extern int icon_item_food_sprite;
extern int icon_item_book_sprite;
extern int icon_item_toy_sprite;
extern int icon_item_prop_sprite;
extern int icon_item_gear_sprite;
extern int icon_coin_sprite;

extern int icon_vig_sprite;
extern int icon_foc_sprite;
extern int icon_spi_sprite;
extern int pip_vig_sprite;
extern int pip_foc_sprite;
extern int pip_spi_sprite;
extern int pip_empty_sprite;

extern int icon_temp_sprite;
extern int icon_co2_sprite;
extern int icon_pm_sprite;
extern int icon_voc_sprite;
extern int icon_nox_sprite;

extern int icon_mask_sprite;
extern int icon_pure_sprite;
extern int icon_uv_sprite;

extern int icon_nosmoke_sprite;
extern int icon_ee_sprite;
extern int icon_aq_ccode_sprite;
extern int icon_cell_sprite;

extern int icon_feed_sprite;
extern int icon_study_sprite;
extern int icon_play_sprite;
extern int icon_deco_sprite;
extern int icon_menu_sprite;

// CURSORS
extern int cursor_sprite;
extern int tile_hl_sprite;

extern int cursor_add_sprite;
extern int tile_hl_add_sprite;

extern int cursor_flip_sprite;
extern int tile_hl_flip_sprite;
extern int tile_mark_flip_sprite;

extern int cursor_remove_sprite;
extern int tile_hl_rm_sprite;
extern int tile_mark_rm_sprite;

extern int button_hl_sprite;
extern int touch_hl_sprite;

// TOOLS
extern int padkaprow_sprite;
extern int sausage_sprite;
extern int coffee_sprite;
extern int salad_sprite;
extern int pill_vig_sprite;
extern int pill_foc_sprite;
extern int pill_spi_sprite;
extern int cigarette_sprite;

extern int book_static_sprite;
extern int book_study_sprite;

extern int toy_duck_sprite;
extern int toy_baseball_sprite;
extern int toy_basketball_sprite;
extern int toy_golf_sprite;
extern int toy_puzzle_sprite;

// KEYS AND GEAR
extern int coin_static_sprite;
extern int coin_world_sprite;

// FIXED PROPS
extern int window_dawn_sprite;
extern int window_day_sprite;
extern int window_night_sprite;
extern int vending_sprite;
extern int arcade_sprite;

// GAMEPLAY PROPS
extern int gpu_sprite;
extern int uv_sprite;
extern int purifier_sprite;

// DECO PROPS
extern int coffeemaker_sprite;
extern int fan_a_sprite;
extern int fan_b_sprite;
extern int lantern_sprite;

extern int table_lg_sprite;
extern int table_sm_sprite;
extern int chair_wood_sprite;
extern int stool_wood_sprite;
extern int stool_stone_sprite;
extern int stool_gold_sprite;

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
extern int flower_spi_sprite;

extern int crystal_blue_lg_sprite;
extern int crystal_green_lg_sprite;
extern int crystal_purple_lg_sprite;

extern int effigy_blue_sprite;
extern int effigy_purple_sprite;
extern int effigy_sea_sprite;

extern int poster_zk_sprite;
extern int pixel_sprite;
extern int padkaprop_sprite;

// PET STATES
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

extern int pet_crit_vig_in_sprite;
extern int pet_crit_vig_sprite;
extern int pet_crit_vig_out_sprite;

extern int pet_crit_foc_in_sprite;
extern int pet_crit_foc_sprite;
extern int pet_crit_foc_out_sprite;

extern int pet_crit_spi_in_sprite;
extern int pet_crit_spi_sprite;
extern int pet_crit_spi_out_sprite;

// PET ACTIONS
extern int pet_eat_in_sprite;
extern int pet_eat_sprite;
extern int pet_eat_out_sprite;

extern int pet_study_in_sprite;
extern int pet_study_sprite;
extern int pet_study_out_sprite;

extern int pet_play_a_sprite;
extern int pet_play_b_sprite;
extern int pet_play_c_sprite;

extern int pet_vig_up_sprite;
extern int pet_foc_up_sprite;
extern int pet_spi_up_sprite;

// PET MOODS
extern int mood_low_vig_sprite;
extern int mood_low_foc_sprite;
extern int mood_low_spi_sprite;
extern int mood_good_sprite;
extern int mood_bad_sprite;

// SNAKE
extern int snake_head_sprite;
extern int snake_body_sprite;
extern int snake_corner_sprite;
extern int snake_tail_sprite;


// MACHINES
extern CAT_animachine_state* pet_asm;

extern CAT_animachine_state AS_idle;
extern CAT_animachine_state AS_walk;
extern CAT_animachine_state AS_crit;

extern CAT_animachine_state AS_adjust_in;
extern CAT_animachine_state AS_approach;
extern CAT_animachine_state AS_adjust_out;

extern CAT_animachine_state AS_eat;
extern CAT_animachine_state AS_study;
extern CAT_animachine_state AS_play;

extern CAT_animachine_state AS_vig_up;
extern CAT_animachine_state AS_foc_up;
extern CAT_animachine_state AS_spi_up;

extern CAT_animachine_state* react_asm;
extern CAT_animachine_state AS_react;

void CAT_sprite_mass_define();

