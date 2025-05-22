#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cat_core.h"
#ifdef CAT_EMBEDDED
#include "lcd_driver.h"
#endif

//////////////////////////////////////////////////////////////////////////
// CONSTANTS AND MACROS

#define CAT_TILE_SIZE 16
#define CAT_GLYPH_WIDTH 8
#define CAT_GLYPH_HEIGHT 12

#define CAT_DRAW_QUEUE_MAX_LENGTH 512
#define CAT_ANIM_TABLE_MAX_LENGTH 512

#ifdef CAT_DESKTOP
#define FRAMEBUFFER_ROW_OFFSET (CAT_get_render_cycle() * CAT_LCD_FRAMEBUFFER_H)

#define ADAPT_DESKTOP_COLOUR(c) c
#define ADAPT_EMBEDDED_COLOUR(c) RGB5652BGR565(c)
#else
#define FRAMEBUFFER_ROW_OFFSET framebuffer_offset_h

#define ADAPT_DESKTOP_COLOUR(c) RGB5652BGR565(c)
#define ADAPT_EMBEDDED_COLOUR(c) c
#endif

#define CAT_BLACK 0x0000
#define CAT_WHITE 0xFFFF
#define CAT_RED 0b1111100000000000
#define CAT_GREEN 0b0000011111100000
#define CAT_BLUE 0b0000000000011111
#define CAT_YELLOW 0b1111111111100000
#define CAT_PURPLE 0b1111100000011111

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

static inline CAT_RGB888 CAT_RGB888_lerp(CAT_RGB888 a, CAT_RGB888 b, float t)
{
	return (CAT_RGB888)
	{
		.r = (1.0f-t) * (float) a.r + t * (float) b.r,
		.g = (1.0f-t) * (float) a.g + t * (float) b.g,
		.b = (1.0f-t) * (float) a.b + t * (float) b.b
	};
}

static inline uint16_t CAT_RGB8882565(CAT_RGB888 c)
{
	return 
	((c.r & 0b11111000) << 8) |
	((c.g & 0b11111100) << 3) |
	(c.b >> 3);
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
	CAT_DRAW_FLAG_DEFAULT = 0,
	CAT_DRAW_FLAG_BOTTOM = 1,
	CAT_DRAW_FLAG_CENTER_X = 2,
	CAT_DRAW_FLAG_CENTER_Y = 4,
	CAT_DRAW_FLAG_REFLECT_X = 8
} CAT_draw_flag;

void CAT_push_draw_flags(int flags);
void CAT_push_draw_colour(uint16_t colour);
void CAT_push_draw_scale(unsigned int);
void CAT_draw_sprite(const CAT_sprite* sprite, int frame_idx, int x, int y);

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
void CAT_circberry(int x, int y, int r, uint16_t c);
void CAT_discberry(int x, int y, int r, uint16_t c);
void CAT_ringberry(int x, int y, int R, int r, uint16_t c, float t);
void CAT_textberry(int x, int y, uint16_t c, int scale, const char* text);
void CAT_textfberry(int x, int y, uint16_t c, int scale, const char* fmt, ...);


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

bool CAT_anim_is_ending(CAT_anim_machine* machine);

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
	int flags;
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

