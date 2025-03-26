#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cat_core.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS AND MACROS

#define CAT_TILE_SIZE 16

#define CAT_DRAW_QUEUE_MAX_LENGTH 512
#define CAT_ANIM_TABLE_MAX_LENGTH 512

#define RGB8882565(r, g, b) (((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3))
#define RGB5652BGR565(c) ((c >> 8) | ((c & 0xff) << 8))

#ifdef CAT_DESKTOP
#define FRAMEBUFFER_ROW_OFFSET (CAT_get_render_cycle() * CAT_LCD_FRAMEBUFFER_H)

#define ADAPT_DESKTOP_COLOUR(c) c
#define ADAPT_EMBEDDED_COLOUR(c) RGB5652BGR565(c)
#else
#include "lcd_driver.h"

#define FRAMEBUFFER_ROW_OFFSET framebuffer_offset_h

#define ADAPT_DESKTOP_COLOUR(c) RGB5652BGR565(c)
#define ADAPT_EMBEDDED_COLOUR(c) c
#endif


//////////////////////////////////////////////////////////////////////////
// THE BERRIER

void CAT_greenberry(int xi, int w, int yi, int h, float t);
void CAT_frameberry(uint16_t c);
void CAT_greyberry(int xi, int w, int yi, int h);
void CAT_lineberry(int xi, int yi, int xf, int yf, uint16_t c);
void CAT_fillberry(int xi, int yi, int w, int h, uint16_t c);
void CAT_strokeberry(int xi, int yi, int w, int h, uint16_t c);
void CAT_rowberry(int x, int y, int w, uint16_t c);


//////////////////////////////////////////////////////////////////////////
// SPRITER

typedef struct
{
	int id;

	const uint16_t* colour_table;
	const uint8_t** frames;
	int frame_count;
	int width;
	int height;

	bool loop;
	bool reverse;
} CAT_sprite;

typedef enum CAT_draw_mode
{
	CAT_DRAW_MODE_DEFAULT = 0,
	CAT_DRAW_MODE_BOTTOM = 1,
	CAT_DRAW_MODE_CENTER_X = 2,
	CAT_DRAW_MODE_CENTER_Y = 4,
	CAT_DRAW_MODE_REFLECT_X = 8
} CAT_draw_mode;

extern CAT_draw_mode draw_mode;

void CAT_draw_sprite(const CAT_sprite* sprite, int frame_idx, int x, int y);


//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

typedef struct CAT_anim_table
{
	int frame_idx[CAT_ANIM_TABLE_MAX_LENGTH];
	bool dirty[CAT_ANIM_TABLE_MAX_LENGTH];
} CAT_anim_table;
extern CAT_anim_table anim_table;

void CAT_anim_table_init();
bool CAT_anim_finished(const CAT_sprite* sprite);
void CAT_anim_reset(const CAT_sprite* sprite);

enum CAT_sprite_layers
{
	BG_LAYER,
	STATICS_LAYER,
	PROPS_LAYER,
	GUI_LAYER
};

typedef struct CAT_draw_job
{
	const CAT_sprite* sprite;
	int frame_idx;
	int layer;
	int x;
	int y;
	int mode;
} CAT_draw_job;

void CAT_draw_queue_clear();
void CAT_draw_queue_insert(int idx, const CAT_sprite* sprite, int frame_idx, int layer, int x, int y, int mode);
int CAT_draw_queue_add(const CAT_sprite* sprite, int frame_idx, int layer, int x, int y, int mode);
void CAT_draw_queue_submit();


//////////////////////////////////////////////////////////////////////////
// ANIMATION MACHINE

typedef struct CAT_animachine_state
{
	enum {ENTER, TICK, EXIT, DONE} signal;

	const CAT_sprite* enter_anim_id;
	const CAT_sprite* tick_anim_id;
	const CAT_sprite* exit_anim_id;
	
	const CAT_sprite* last;
	struct CAT_animachine_state* next;
} CAT_animachine_state;

void CAT_animachine_init(CAT_animachine_state* state, const CAT_sprite* enai, const CAT_sprite* tiai, const CAT_sprite* exai);
void CAT_animachine_transition(CAT_animachine_state** spp, CAT_animachine_state* next);
const CAT_sprite* CAT_animachine_tick(CAT_animachine_state** pp);
void CAT_animachine_kill(CAT_animachine_state** spp);

bool CAT_animachine_is_in(CAT_animachine_state** spp, CAT_animachine_state* state);
bool CAT_animachine_is_ticking(CAT_animachine_state** spp);
bool CAT_animachine_is_done(CAT_animachine_state** spp);


//////////////////////////////////////////////////////////////////////////
// MESHES

typedef struct CAT_mesh
{
	const char* path;
	float* verts;
	int n_verts;
	int* faces;
	int n_faces;
} CAT_mesh;


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

