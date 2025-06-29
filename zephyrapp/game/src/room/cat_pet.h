#pragma once

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_render.h"

#define CAT_LIFE_TICK_PERIOD CAT_DAY_SECONDS
#define CAT_STAT_TICK_PERIOD (CAT_DAY_SECONDS / 4)
#define CAT_PET_MILK_COOLDOWN (CAT_MINUTE_SECONDS)

typedef struct __attribute__((__packed__))
{
	uint64_t last_stat_time;
	uint64_t last_life_time;

	uint64_t last_milk_time;
	uint8_t times_milked_since_producing;
	uint8_t milks_produced_today;
} CAT_pet_timing_state;
void CAT_pet_export_timing_state(CAT_pet_timing_state* out);
void CAT_pet_import_timing_state(CAT_pet_timing_state* in);

typedef struct CAT_pet
{
	char name[64];

	uint8_t lifespan;
	uint64_t birthday;
	uint64_t deathday;
	uint16_t incarnations;

	uint8_t level;
	uint32_t xp;
	
	uint8_t vigour;
	uint8_t focus;
	uint8_t spirit;

	CAT_vec2 pos;
	CAT_vec2 vel;
	int rot;
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

void CAT_pet_tick();

int CAT_pet_days_alive();
bool CAT_pet_is_dead();
void CAT_pet_reincarnate();
void CAT_pet_post_death_report();
void CAT_pet_dismiss_death_report();
bool CAT_pet_is_death_report_posted();
void CAT_MS_death_report(CAT_machine_signal signal);
void CAT_render_death_report();
