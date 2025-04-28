#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cat_core.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS AND MACROS

#define CAT_TILE_SIZE 16

#define CAT_DRAW_QUEUE_MAX_LENGTH 512
#define CAT_ANIM_TABLE_MAX_LENGTH 512

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
void CAT_pixberry(int x, int y, uint16_t c);


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

typedef enum
{
	CAT_DRAW_FLAG_DEFAULT = 0,
	CAT_DRAW_FLAG_BOTTOM = 1,
	CAT_DRAW_FLAG_CENTER_X = 2,
	CAT_DRAW_FLAG_CENTER_Y = 4,
	CAT_DRAW_FLAG_REFLECT_X = 8,
	CAT_DRAW_FLAG_OUTLINE = 16
} CAT_draw_flag;

extern CAT_draw_flag draw_flags;

void CAT_draw_sprite(const CAT_sprite* sprite, int frame_idx, int x, int y);


//////////////////////////////////////////////////////////////////////////
// ANIMATOR

void CAT_animator_init();
void CAT_animator_tick();
int CAT_animator_get_frame(const CAT_sprite* sprite);


//////////////////////////////////////////////////////////////////////////
// ANIM GRAPH

typedef struct CAT_anim_state
{
	const CAT_sprite* enter_sprite;
	const CAT_sprite* tick_sprite;
	const CAT_sprite* exit_sprite;
} CAT_anim_state;

typedef struct CAT_anim_machine
{
	CAT_anim_state* state;
	CAT_anim_state* next;
	enum {ENTER, TICK, EXIT} signal;
} CAT_anim_machine;

void CAT_anim_transition(CAT_anim_machine* machine, CAT_anim_state* next);
void CAT_anim_tick(CAT_anim_machine* machine);
const CAT_sprite* CAT_anim_read(CAT_anim_machine* machine);

void CAT_anim_kill(CAT_anim_machine* machine);
bool CAT_anim_is_dead(CAT_anim_machine* machine);

bool CAT_anim_is_in(CAT_anim_machine* machine, CAT_anim_state* state);
bool CAT_anim_is_ticking(CAT_anim_machine* machine);

//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

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

extern CAT_anim_state AS_idle;
extern CAT_anim_state AS_walk;
extern CAT_anim_state AS_crit;

extern CAT_anim_state AS_approach;

extern CAT_anim_state AS_eat;
extern CAT_anim_state AS_study;
extern CAT_anim_state AS_play;

extern CAT_anim_state AS_vig_up;
extern CAT_anim_state AS_foc_up;
extern CAT_anim_state AS_spi_up;

extern CAT_anim_state AS_react;

extern CAT_anim_state AS_pounce;

