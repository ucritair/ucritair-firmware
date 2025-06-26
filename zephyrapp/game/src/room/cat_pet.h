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
	char name[64];

	unsigned int lifetime;
	unsigned int lifespan;
	unsigned int incarnations;
	uint64_t birthday;
	uint64_t deathday;

	unsigned int level;
	unsigned int xp;
	
	unsigned int vigour;
	unsigned int focus;
	unsigned int spirit;

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
} CAT_pet;
extern CAT_pet pet;
extern CAT_anim_machine AM_pet;
extern CAT_anim_machine AM_mood;

void CAT_pet_init();
void CAT_pet_update_animations();
void CAT_pet_settle();
void CAT_pet_walk();
void CAT_pet_react();

bool CAT_pet_seek(CAT_vec2 targ);
void CAT_pet_face(CAT_vec2 targ);

void CAT_pet_gain_xp(int xp);
void CAT_pet_stat(int ticks);
void CAT_pet_life(int ticks);

void CAT_pet_tick();

bool CAT_pet_is_dead();
void CAT_pet_reincarnate();
void CAT_pet_post_death_report();
void CAT_pet_dismiss_death_report();
void CAT_MS_death_report(CAT_machine_signal signal);
void CAT_render_death_report();
