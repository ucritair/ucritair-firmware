#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cat_core.h"

#include "sprite_assets.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS AND MACROS

#define CAT_ATLAS_MAX_LENGTH 512

#define CAT_DRAW_QUEUE_MAX_LENGTH 512

#define CAT_ANIM_TABLE_MAX_LENGTH CAT_ATLAS_MAX_LENGTH
#define CAT_ANIM_QUEUE_MAX_LENGTH CAT_ATLAS_MAX_LENGTH

#define CAT_TILE_SIZE 16

#define RGB8882565(r, g, b) ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3)


//////////////////////////////////////////////////////////////////////////
// THE BERRIER

void CAT_greenberry(int xi, int w, int yi, int h, float t);
void CAT_frameberry(uint16_t c);
void CAT_greyberry(int xi, int w, int yi, int h);
void CAT_lineberry(int xi, int yi, int xf, int yf, uint16_t c);
void CAT_fillberry(int xi, int yi, int w, int h, uint16_t c);
void CAT_strokeberry(int xi, int yi, int w, int h, uint16_t c);


//////////////////////////////////////////////////////////////////////////
// ATLAS

typedef struct {
	const uint16_t* color_table;
	const uint8_t** frames;
} CAT_baked_sprite;

typedef struct CAT_sprite
{
#ifdef CAT_DESKTOP
	const uint16_t* pixels;
#endif
	int width;
	int height;
	int frame_count;
} CAT_sprite;

typedef struct CAT_atlas
{
	CAT_sprite data[CAT_ATLAS_MAX_LENGTH];
	int length;
} CAT_atlas;
extern CAT_atlas atlas;

CAT_sprite* CAT_sprite_get(int sprite_id);


//////////////////////////////////////////////////////////////////////////
// SPRITER

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

typedef struct CAT_anim_table
{
	int frame_idx[CAT_ATLAS_MAX_LENGTH];
	bool loop[CAT_ATLAS_MAX_LENGTH];
	bool reverse[CAT_ATLAS_MAX_LENGTH];
	bool dirty[CAT_ATLAS_MAX_LENGTH];

	float timer;
} CAT_anim_table;
extern CAT_anim_table anim_table;

void CAT_anim_table_init();
void CAT_anim_toggle_loop(int sprite_id, bool toggle);
void CAT_anim_toggle_reverse(int sprite_id, bool toggle);
bool CAT_anim_finished(int sprite_id);
void CAT_anim_reset(int sprite_id);

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

void CAT_draw_queue_insert(int idx, int sprite_id, int frame_idx, int layer, int x, int y, int mode);
int CAT_draw_queue_add(int sprite_id, int frame_idx, int layer, int x, int y, int mode);
void CAT_draw_queue_submit(int cycle);


//////////////////////////////////////////////////////////////////////////
// ANIMATION MACHINE

typedef struct CAT_animachine_state
{
	enum {ENTER, TICK, EXIT, DONE} signal;

	int enter_anim_id;
	int tick_anim_id;
	int exit_anim_id;
	int last;

	struct CAT_animachine_state* next;
} CAT_animachine_state;

void CAT_animachine_init(CAT_animachine_state* state, int enai, int tiai, int exai);
void CAT_animachine_transition(CAT_animachine_state** spp, CAT_animachine_state* next);
int CAT_animachine_tick(CAT_animachine_state** pp);
void CAT_animachine_kill(CAT_animachine_state** spp);

bool CAT_animachine_is_in(CAT_animachine_state** spp, CAT_animachine_state* state);
bool CAT_animachine_is_ticking(CAT_animachine_state** spp);
bool CAT_animachine_is_done(CAT_animachine_state** spp);


//////////////////////////////////////////////////////////////////////////
// DECLARATIONS

// MACHINES
extern CAT_animachine_state* pet_asm;

extern CAT_animachine_state AS_idle;
extern CAT_animachine_state AS_walk;
extern CAT_animachine_state AS_crit;

extern CAT_animachine_state AS_approach;

extern CAT_animachine_state AS_eat;
extern CAT_animachine_state AS_study;
extern CAT_animachine_state AS_play;

extern CAT_animachine_state AS_vig_up;
extern CAT_animachine_state AS_foc_up;
extern CAT_animachine_state AS_spi_up;

extern CAT_animachine_state* react_asm;
extern CAT_animachine_state AS_react;

void CAT_sprite_mass_define();

