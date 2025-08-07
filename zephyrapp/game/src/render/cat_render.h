#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cat_core.h"
#include "cat_colours.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS AND MACROS

#ifdef CAT_DESKTOP
#define CAT_LCD_FRAMEBUFFER_OFFSET (CAT_get_render_cycle() * CAT_LCD_FRAMEBUFFER_H)

#define ADAPT_DESKTOP_COLOUR(c) c
#define ADAPT_EMBEDDED_COLOUR(c) RGB5652BGR565(c)
#else
#include "lcd_driver.h"
#define CAT_LCD_FRAMEBUFFER_OFFSET framebuffer_offset_h

#define ADAPT_DESKTOP_COLOUR(c) RGB5652BGR565(c)
#define ADAPT_EMBEDDED_COLOUR(c) c
#endif

//////////////////////////////////////////////////////////////////////////
// COLOUR

typedef union
{
	struct
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};

	uint8_t channels[3];
} CAT_RGB888;

static inline CAT_RGB888 CAT_RGB24(uint8_t r, uint8_t g, uint8_t b)
{
	return (CAT_RGB888)
	{
		.r = r,
		.g = g,
		.b = b
	};
}

static inline CAT_RGB888 CAT_RGB24_lerp(CAT_RGB888 a, CAT_RGB888 b, float t)
{
	return (CAT_RGB888)
	{
		.r = (1.0f-t) * (float) a.r + t * (float) b.r,
		.g = (1.0f-t) * (float) a.g + t * (float) b.g,
		.b = (1.0f-t) * (float) a.b + t * (float) b.b
	};
}

static inline uint16_t CAT_RGB24216(CAT_RGB888 c)
{
	return 
	((c.r & 0b11111000) << 8) |
	((c.g & 0b11111100) << 3) |
	(c.b >> 3);
}

static inline CAT_RGB888 CAT_RGB16224(uint16_t c)
{
	uint8_t r8 = (c & 0b1111100000000000) >> 11;
	uint8_t g8 = (c & 0b0000011111100000) >> 5;
	uint8_t b8 = c & 0b0000000000011111;
	uint16_t r16 = clamp(255 * r8 / 31, 0, 255);
	uint16_t g16 = clamp(255 * g8 / 63, 0, 255);
	uint16_t b16 = clamp(255 * b8 / 31, 0, 255);
	return CAT_RGB24(r16, g16, b16);
}


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
	CAT_DRAW_FLAG_NONE = 0,
	CAT_DRAW_FLAG_BOTTOM = (1 << 0),
	CAT_DRAW_FLAG_CENTER_X = (1 << 1),
	CAT_DRAW_FLAG_CENTER_Y = (1 << 2),
	CAT_DRAW_FLAG_REFLECT_X = (1 << 3)
} CAT_draw_flag;

typedef enum
{
	CAT_SPRITE_OVERRIDE_NONE = 0,
	CAT_SPRITE_OVERRIDE_FLAGS = (1 << 0),
	CAT_SPRITE_OVERRIDE_COLOUR = (1 << 1),
	CAT_SPRITE_OVERRIDE_SCALE = (1 << 2),
	CAT_SPRITE_OVERRIDE_MASK = (1 << 3)
} CAT_sprite_override;

void CAT_set_sprite_flags(uint64_t flags);
void CAT_set_sprite_colour(uint16_t colour);
void CAT_set_sprite_scale(uint8_t scale);
void CAT_set_sprite_mask(int x0, int y0, int x1, int y1);

void CAT_draw_sprite(const CAT_sprite* sprite, int frame_idx, int x, int y);
void CAT_draw_sprite_raw(const CAT_sprite* sprite, int frame_idx, int x, int y);
void CAT_draw_background(const CAT_sprite* sprite, int frame_idx, int y);
void CAT_draw_tile(const CAT_sprite* sprite, int frame_idx, int x, int y);
void CAT_draw_tile_alpha(const CAT_sprite* sprite, int frame_idx, int x, int y);

//////////////////////////////////////////////////////////////////////////
// THE BERRIER

void CAT_frameberry(uint16_t c);
void CAT_lineberry(int xi, int yi, int xf, int yf, uint16_t c);
void CAT_fillberry(int xi, int yi, int w, int h, uint16_t c);
void CAT_strokeberry(int xi, int yi, int w, int h, uint16_t c);
void CAT_pixberry(int x, int y, uint16_t c);
void CAT_circberry(int x, int y, int r, uint16_t c);
void CAT_discberry(int x, int y, int r, uint16_t c);
void CAT_annulusberry(int x, int y, int R, int r, uint16_t c, float t, float shift);


//////////////////////////////////////////////////////////////////////////
// ANIMATOR

void CAT_animator_init();
void CAT_animator_tick();
int CAT_animator_get_frame(const CAT_sprite* sprite);
void CAT_animator_reset(const CAT_sprite* sprite);


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

bool CAT_anim_is_ending(CAT_anim_machine* machine);


//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

#define CAT_DRAW_QUEUE_MAX_LENGTH 256

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
	int flags;
} CAT_draw_job;

void CAT_draw_queue_clear();
void CAT_draw_queue_insert(int idx, const CAT_sprite* sprite, int frame_idx, int layer, int x, int y, int mode);
int CAT_draw_queue_add(const CAT_sprite* sprite, int frame_idx, int layer, int x, int y, int mode);
void CAT_draw_queue_submit();


//////////////////////////////////////////////////////////////////////////
// MESHES AND POLYLINES

typedef struct
{
	float* verts;
	int n_verts;
	uint8_t* faces;
	int n_faces;
} CAT_mesh;

typedef struct
{
	uint8_t* verts;
	uint16_t vert_count;
	uint16_t* edges;
	uint16_t edge_count;
} CAT_mesh2d;

void CAT_draw_mesh2d(const CAT_mesh2d* mesh, int x, int y, uint16_t c);

typedef enum
{
	CAT_POLY_MODE_LINES,
	CAT_POLY_MODE_LINE_STRIP,
	CAT_POLY_MODE_LINE_LOOP
} CAT_poly_mode;

void CAT_draw_polyline(int x, int y, int16_t* poly, int count, uint16_t c, CAT_poly_mode mode);


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

