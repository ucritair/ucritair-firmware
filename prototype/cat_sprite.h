#ifndef CAT_SPRITE_H
#define CAT_SPRITE_H

#include <stdint.h>
#include <stdbool.h>

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

	CAT_sprite table[512];
	int length;
} CAT_atlas;
extern CAT_atlas atlas;

void CAT_atlas_init(const char* path);
int CAT_atlas_add(int x, int y, int w, int h);

typedef enum CAT_draw_mode
{
	CAT_DRAW_MODE_DEFAULT = 0,
	CAT_DRAW_MODE_BOTTOM = 1,
	CAT_DRAW_MODE_CENTER_X = 2,
	CAT_DRAW_MODE_CENTER_Y = 4,
	CAT_DRAW_MODE_REFLECT_X = 8,
	CAT_DRAW_MODE_WIREFRAME = 16, 
} CAT_draw_mode;

typedef struct CAT_spriter
{
	uint16_t* frame;
	int mode;
} CAT_spriter;
extern CAT_spriter spriter;

void CAT_spriter_init();
void CAT_draw_sprite(int x, int y, int key);
void CAT_draw_tiles(int y_t, int h_t, int key);

typedef struct CAT_draw_job
{
	int key;
	int layer;
	int x;
	int y;
	int mode;
} CAT_draw_job;

typedef struct CAT_draw_queue
{
	CAT_draw_job jobs[256];
	int length;
} CAT_draw_queue;
extern CAT_draw_queue draw_queue;

void CAT_draw_queue_init();
void CAT_draw_queue_add(int key, int layer, int x, int y, int mode);
void CAT_draw_queue_submit();

typedef struct CAT_anim
{
	int frames[16];
	int length;
	int idx;
	int looping;
} CAT_anim;

void CAT_anim_init(CAT_anim* anim);
void CAT_anim_add(CAT_anim* anim, int key);
void CAT_anim_tick(CAT_anim* anim);
int CAT_anim_frame(CAT_anim* anim);

typedef struct CAT_animator
{
	CAT_anim* anims[256];
	int length;
	
	float period;
	float timer;
 } CAT_animator;
extern CAT_animator animator;

void CAT_animator_init();
void CAT_animator_add(CAT_anim* anim);
void CAT_animator_tick(float dt);

#endif
