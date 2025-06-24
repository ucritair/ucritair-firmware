#pragma once

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_render.h"

#define CAT_LIFE_TICK_TIME (CAT_DAY_SECONDS)
#define CAT_STAT_TICK_TIME (CAT_DAY_SECONDS / 4)
#define CAT_PET_PET_COOLDOWN (CAT_MINUTE_SECONDS)
#define CAT_PET_WALK_COOLDOWN 4
#define CAT_PET_REACT_TIME 2

typedef struct CAT_pet
{
	unsigned int vigour;
	unsigned int focus;
	unsigned int spirit;
	unsigned int lifetime;
	unsigned int lifespan;
	
	unsigned int xp;
	unsigned int level;

	CAT_vec2 pos;
	CAT_vec2 vel;
	int rot;
	
	float stat_timer;
	float life_timer;

	float walk_timer;
	float react_timer;

	unsigned int times_pet;
	float petting_timer;
	unsigned int times_milked;

	char name[64];
} CAT_pet;
extern CAT_pet pet;
extern CAT_anim_machine AM_pet;
extern CAT_anim_machine AM_mood;

void CAT_pet_init();
void CAT_pet_reanimate();
void CAT_pet_settle();
void CAT_pet_walk();
void CAT_pet_react();

bool CAT_pet_seek(CAT_vec2 targ);
void CAT_pet_face(CAT_vec2 targ);

void CAT_pet_gain_xp(int xp);
void CAT_pet_stat(int ticks);
void CAT_pet_life(int ticks);

void CAT_pet_tick();
