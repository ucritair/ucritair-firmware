#pragma once

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_render.h"

#define CAT_MIN_SECS 60
#define CAT_HOUR_SECS 3600
#define CAT_DAY_SECS 86400

#define CAT_LIFE_TICK_SECS (CAT_DAY_SECS)
#define CAT_STAT_TICK_SECS (CAT_DAY_SECS / 4)
#define CAT_PET_COOLDOWN_SECS (CAT_MIN_SECS)

typedef struct CAT_pet
{
	unsigned int vigour;
	unsigned int focus;
	unsigned int spirit;
	unsigned int lifetime;
	unsigned int xp;
	unsigned int level;

	CAT_vec2 pos;
	CAT_vec2 vel;
	int rot;
	
	int stat_timer_id;
	int life_timer_id;

	int walk_timer_id;
	int react_timer_id;

	unsigned int times_pet;
	int petting_timer_id;
	unsigned int times_milked;

	char name[64];
} CAT_pet;
extern CAT_pet pet;
extern CAT_anim_machine AM_pet;
extern CAT_anim_machine AM_mood;

void CAT_pet_init();
void CAT_pet_reanimate();
void CAT_pet_settle();

bool CAT_pet_seek(CAT_vec2 targ);
void CAT_pet_face(CAT_vec2 targ);

void CAT_pet_stat(int ticks);
void CAT_pet_life(int ticks);

void CAT_pet_tick();
