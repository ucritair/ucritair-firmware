#pragma once

#include "cat_machine.h"
#include "cat_math.h"

#define CAT_MIN_SECS 60
#define CAT_HOUR_SECS 3600
#define CAT_DAY_SECS 86400

#define CAT_LIFE_TICK_SECS (CAT_DAY_SECS)
#define CAT_STAT_TICK_SECS (CAT_DAY_SECS / 4)
#define CAT_PET_COOLDOWN_SECS (CAT_MIN_SECS)

typedef struct CAT_pet
{
	int vigour;
	int focus;
	int spirit;
	int lifetime;
	int xp;
	int level;

	CAT_vec2 pos;
	CAT_vec2 dir;
	bool left;
	
	int stat_timer_id;
	int life_timer_id;

	int walk_timer_id;
	int react_timer_id;

	int times_pet;
	int petting_timer_id;
	int times_milked;

	char name[64];
} CAT_pet;
extern CAT_pet pet;

void CAT_pet_init();
void CAT_pet_reanimate();
void CAT_pet_settle();

bool CAT_pet_seek(CAT_vec2 targ);
void CAT_pet_reposition();

void CAT_pet_stat(int ticks);
void CAT_pet_life(int ticks);

void CAT_pet_tick(bool capture_input);
void CAT_render_pet(int cycle);
