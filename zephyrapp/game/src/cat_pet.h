#pragma once

#include "cat_machine.h"
#include "cat_math.h"

#ifdef CAT_DESKTOP
#define CAT_STAT_TICK_SECS 10
#define CAT_LIFE_TICK_SECS 10
#else 
#define CAT_STAT_TICK_SECS 7200
#define CAT_LIFE_TICK_SECS 86400
#endif

typedef struct CAT_pet
{
	int vigour;
	int focus;
	int spirit;
	int lifetime;

	CAT_vec2 pos;
	CAT_vec2 dir;
	bool left;
	
	int stat_timer_id;
	int life_timer_id;
	int walk_timer_id;
	int react_timer_id;
} CAT_pet;
extern CAT_pet pet;

void CAT_pet_stat(int ticks);
void CAT_pet_life(int ticks);
void CAT_pet_use(int item_id);
bool CAT_pet_is_critical();
void CAT_pet_reanimate();
void CAT_pet_settle();
bool CAT_pet_seek(CAT_vec2 targ);
void CAT_pet_init();

void CAT_pet_background_tick(bool capture_input);
void CAT_render_pet(int cycle);
