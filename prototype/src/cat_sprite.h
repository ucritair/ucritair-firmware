#ifndef CAT_SPRITE_H
#define CAT_SPRITE_H

#include <stdint.h>
#include <stdbool.h>

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

typedef struct CAT_sprite
{
	int x;
	int y;
	int width;
	int height;
} CAT_sprite;

typedef struct CAT_atlas
{
	int width;
	int height;
	uint16_t* rgb;
	bool* alpha;

	CAT_sprite table[CAT_ATLAS_MAX_LENGTH];
	int length;
} CAT_atlas;
extern CAT_atlas atlas;

void CAT_atlas_init(const char* path);
int CAT_atlas_add(int x, int y, int w, int h);
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
	uint16_t* frame;
	int mode;
} CAT_spriter;
extern CAT_spriter spriter;

void CAT_spriter_init();
void CAT_draw_sprite(int x, int y, int sprite_id);
void CAT_draw_tiles(int y_t, int h_t, int sprite_id);
void CAT_spriter_cleanup();

//////////////////////////////////////////////////////////////////////////
// DRAW QUEUE

typedef struct CAT_draw_job
{
	int sprite_id;
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

void CAT_draw_queue_init();
void CAT_draw_queue_add(int sprite_id, int layer, int x, int y, int mode);
void CAT_draw_queue_submit();

//////////////////////////////////////////////////////////////////////////
// ANIMATIONS

typedef struct CAT_anim
{
	int frames[CAT_ANIM_MAX_LENGTH];
	int length;
	int idx;
	int looping;
} CAT_anim;

typedef struct CAT_anim_table
{
	CAT_anim data[CAT_ANIM_TABLE_MAX_LENGTH];
	int length;
} CAT_anim_table;
extern CAT_anim_table anim_table;

void CAT_anim_table_init();
int CAT_anim_init(int* sprite_id, int length, bool looping);
CAT_anim* CAT_anim_get(int anim_id);
void CAT_anim_tick(int anim_id);
int CAT_anim_frame(int anim_id);

//////////////////////////////////////////////////////////////////////////
// ANIM QUEUE

typedef struct CAT_anim_job
{
	int anim_id;
	int layer;
	int x;
	int y;
	int mode;
} CAT_anim_job;

typedef struct CAT_anim_queue
{
	CAT_anim_job jobs[CAT_ANIM_QUEUE_MAX_LENGTH];
	int length;
	
	float period;
	float timer;
 } CAT_anim_queue;
extern CAT_anim_queue anim_queue;

void CAT_anim_queue_init();
void CAT_anim_queue_add(int anim_id, int layer, int x, int y, int mode);
void CAT_anim_queue_submit();

//////////////////////////////////////////////////////////////////////////
// ID DECLARATIONS

extern int wall_sprite_id[3];
extern int floor_sprite_id[3];
extern int pet_sprite_id[13];
extern int fed_sprite_id[10];
extern int studied_sprite_id[10];
extern int played_sprite_id[10];
extern int low_vigour_sprite_id[3];
extern int low_spirit_sprite_id[3];
extern int anger_sprite_id[3];
extern int vending_sprite_id[13];
extern int pot_sprite_id[7];
extern int chair_sprite_id[4];
extern int table_sprite_id;
extern int coffee_sprite_id[2];
extern int device_sprite_id;
extern int seed_sprite_id[6];

extern int cursor_sprite_id[4];
extern int feed_button_sprite_id;
extern int study_button_sprite_id;
extern int play_button_sprite_id;
extern int ring_hl_sprite_id;

extern int panel_sprite_id[9];
extern int glyph_sprite_id[91];
extern int a_sprite_id;
extern int b_sprite_id;
extern int enter_sprite_id;
extern int exit_sprite_id;
extern int select_sprite_id;
extern int arrow_sprite_id;
extern int item_sprite_id;
extern int cell_sprite_id[4];
extern int vigour_sprite_id;
extern int focus_sprite_id;
extern int spirit_sprite_id;
	
extern int idle_anim_id;
extern int walk_anim_id;
extern int death_anim_id;
extern int fed_anim_id;
extern int studied_anim_id;
extern int played_anim_id;
extern int vending_anim_id;
extern int pot_anim_id;
extern int chair_anim_id;
extern int coffee_anim_id;

extern int cursor_anim_id;

void CAT_sprite_mass_define();

#endif
