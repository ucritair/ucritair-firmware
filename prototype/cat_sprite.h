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
int CAT_atlas_add(CAT_sprite sprite);

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
void CAT_draw_sprite(int x, int y, int key);

typedef struct CAT_anim
{
	int frame_count;
	int frames[8];
	int idx;
	int looping;
} CAT_anim;

void CAT_anim_init(CAT_anim* anim);
void CAT_anim_add(CAT_anim* anim, int key);
void CAT_anim_tick(CAT_anim* anim);

typedef struct CAT_anim_cmd
{
	CAT_anim* anim;
	int layer;
	int x;
	int y;
	int mode;
} CAT_anim_cmd;

void CAT_anim_cmd_init(CAT_anim_cmd* cmd, CAT_anim* anim, int layer, int x, int y, int mode);

typedef struct CAT_anim_queue
{
	CAT_anim_cmd items[1024];
	int length;
	
	float period;
	float timer;
} CAT_anim_queue;
extern CAT_anim_queue anim_queue;

void CAT_anim_queue_init();
void CAT_anim_queue_add(CAT_anim_cmd cmd);
void CAT_anim_queue_tick(float dt);
void CAT_anim_queue_draw();

#endif
