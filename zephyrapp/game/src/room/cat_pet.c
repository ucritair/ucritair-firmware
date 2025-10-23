#include "cat_pet.h"

#include <stddef.h>
#include "cat_core.h"
#include "cat_render.h"
#include <stdio.h>
#include <math.h>
#include "cat_item.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_gui.h"
#include <string.h>
#include "cat_aqi.h"
#include "sprite_assets.h"
#include "cat_persist.h"
#include "item_assets.h"

static CAT_pet_timing_state timing_state = {0};
void CAT_pet_export_timing_state(CAT_pet_timing_state* out)
{
	memcpy(out, &timing_state, sizeof(CAT_pet_timing_state));
}
void CAT_pet_import_timing_state(CAT_pet_timing_state* in)
{
	memcpy(&timing_state, in, sizeof(CAT_pet_timing_state));
}

CAT_pet pet;
CAT_anim_machine AM_pet =
{
	.state = NULL,
	.next = NULL,
	.signal = ENTER
};
CAT_anim_machine AM_mood =
{
	.state = NULL,
	.next = NULL,
	.signal = ENTER
};

bool is_critical()
{
	return 
	pet.vigour <= 0 ||
	pet.focus <= 0 ||
	pet.spirit <= 0;
}

void CAT_pet_init()
{	
	timing_state = (CAT_pet_timing_state)
	{
		.last_life_time = CAT_get_RTC_now(),
		.last_milk_time = CAT_get_RTC_now(),
		.last_stat_time = CAT_get_RTC_now(),
		.milks_produced_today = 0,
		.times_milked_since_producing = 0
	};

	strncpy(pet.name, CAT_DEFAULT_PET_NAME, sizeof(pet.name));

	pet.lifespan = 30;
	pet.lifetime = 0;
	pet.incarnations = 1;

	pet.xp = 0;
	pet.level = 0;

	pet.vigour = 12;
	pet.focus = 12;
	pet.spirit = 12;

	pet.pos = (CAT_vec2) {120, 200};
	pet.vel = (CAT_vec2) {0, 0};
	pet.rot = 1;
}

void CAT_pet_update_animations()
{
	AS_idle.enter_sprite = NULL;
	AS_idle.tick_sprite = &pet_idle_sprite;
	AS_idle.exit_sprite = NULL;

	AS_walk.enter_sprite = NULL;
	AS_walk.tick_sprite = &pet_walk_sprite;
	AS_walk.exit_sprite = NULL;

	AS_crit.enter_sprite = &pet_crit_vig_in_sprite;
	AS_crit.tick_sprite = &pet_crit_vig_sprite;
	AS_crit.exit_sprite = &pet_crit_vig_out_sprite;

	AS_react.tick_sprite = &mood_good_sprite;

	if(pet.vigour >= 9 || pet.focus >= 9 || pet.spirit >= 9)
	{
		if(pet.vigour >= 9)
		{
			AS_idle.tick_sprite = &pet_idle_high_vig_sprite;
			AS_walk.tick_sprite = &pet_walk_high_vig_sprite;
		}
		else if(pet.spirit >= 9)
		{
			AS_idle.tick_sprite = &pet_idle_high_spi_sprite;
			AS_walk.tick_sprite = &pet_walk_high_spi_sprite;
		}
		else
		{
			AS_idle.tick_sprite = &pet_idle_high_foc_sprite;
			AS_walk.tick_sprite = &pet_walk_high_foc_sprite;
		}
	}
	if(pet.vigour <= 3 || pet.focus <= 3 || pet.spirit <= 3)
	{
		if(pet.vigour <= 3)
		{
			AS_idle.tick_sprite = &pet_idle_low_vig_sprite;
			AS_walk.tick_sprite = &pet_walk_low_vig_sprite;	
			AS_react.tick_sprite = &mood_low_vig_sprite;
		}
		else if(pet.spirit <= 3)
		{
			AS_idle.tick_sprite = &pet_idle_low_spi_sprite;
			AS_walk.tick_sprite = &pet_walk_low_spi_sprite;
			AS_react.tick_sprite = &mood_low_spi_sprite;
		}
		else
		{
			AS_idle.tick_sprite = &pet_idle_low_foc_sprite;
			AS_walk.tick_sprite = &pet_walk_low_foc_sprite;
			AS_react.tick_sprite = &mood_low_foc_sprite;
		}
	}
	if(is_critical())
	{
		if(pet.vigour <= 0)
		{
			AS_crit.enter_sprite = &pet_crit_vig_in_sprite;
			AS_crit.tick_sprite = &pet_crit_vig_sprite;
			AS_crit.exit_sprite = &pet_crit_vig_out_sprite;
		}
		else if(pet.spirit <= 0)
		{
			AS_crit.enter_sprite = &pet_crit_spi_in_sprite;
			AS_crit.tick_sprite = &pet_crit_spi_sprite;
			AS_crit.exit_sprite = &pet_crit_spi_out_sprite;
		}
		else
		{
			AS_crit.enter_sprite = &pet_crit_foc_in_sprite;
			AS_crit.tick_sprite = &pet_crit_foc_sprite;
			AS_crit.exit_sprite = &pet_crit_foc_out_sprite;
		}
		AS_react.tick_sprite = &mood_bad_sprite;
	}
}

void CAT_pet_settle()
{
	if(is_critical())
		CAT_anim_transition(&AM_pet, &AS_crit);
	else
		CAT_anim_transition(&AM_pet, &AS_idle);
}

bool CAT_pet_seek(CAT_vec2 targ)
{
	CAT_vec2 line = CAT_vec2_sub(targ, pet.pos);
	float dist = sqrtf(CAT_vec2_mag2(line));
	CAT_vec2 dir = CAT_vec2_mul(line, 1.0f/dist);

	float speed = 48.0f;
	float step = speed * CAT_get_delta_time_s();

	if(dist < step)
	{
		pet.vel = (CAT_vec2) {0, 0};
		pet.pos = targ;
		return true;
	}
	else
	{
		pet.rot = dir.x > 0 ? -1 : 1;
		pet.vel = CAT_vec2_mul(dir, speed);
		pet.pos = CAT_vec2_add(pet.pos, CAT_vec2_mul(pet.vel, CAT_get_delta_time_s()));	
		return false;
	}
}

void CAT_pet_face(CAT_vec2 targ)
{
	pet.rot = targ.x > pet.pos.x ? -1 : 1;
}

void CAT_pet_change_XP(int delta)
{
	if(CAT_pet_is_dead())
		return;

	pet.xp += delta;
	pet.xp = CAT_clamp(pet.xp + delta, 0, level_cutoffs[CAT_NUM_LEVELS-1]);

	int cutoff = level_cutoffs[pet.level];
	if(pet.xp >= cutoff)
	{
		pet.level = CAT_min(pet.level+1, CAT_NUM_LEVELS-1);
		pet.xp -= cutoff;
	}
}

void CAT_pet_change_vigour(int delta)
{
	if(CAT_pet_is_dead())
		return;

	pet.vigour = CAT_clamp(pet.vigour+delta, 0, 12);
}

void CAT_pet_change_focus(int delta)
{
	if(CAT_pet_is_dead())
		return;

	pet.focus = CAT_clamp(pet.focus+delta, 0, 12);
}

void CAT_pet_change_spirit(int delta)
{
	if(CAT_pet_is_dead())
		return;

	pet.spirit = CAT_clamp(pet.spirit+delta, 0, 12);
}

static CAT_vec2 destination = {120, 200};

void milk_proc()
{
	CAT_inventory_add(food_milk_item, 1);
}

void apply_life_ticks(int ticks)
{
	if(CAT_pet_is_dead())
		return;
	if(persist_flags & CAT_PERSIST_CONFIG_FLAG_PAUSE_CARE)
		return;

	pet.lifetime += ticks;
	
	for(int i = 0; i < ticks; i++)
	{
		int xp_delta = roundf(((pet.vigour + pet.focus + pet.spirit) / 3.0f) * 50.0f);
		CAT_pet_change_XP(xp_delta);

		timing_state.times_milked_since_producing = 0;
	}
}

void apply_stat_ticks(int ticks)
{
	if(CAT_pet_is_dead())
		return;
	if(persist_flags & CAT_PERSIST_CONFIG_FLAG_PAUSE_CARE)
		return;

	for(int i = 0; i < ticks; i++)
	{
		CAT_pet_change_vigour(-1);
		CAT_pet_change_focus(-1);
		CAT_pet_change_spirit(-1);

		if(pet.vigour <= 0 || pet.focus <= 0 || pet.spirit <= 0)
		{
			int xp_delta = roundf(level_cutoffs[pet.level] * 0.1f);
			CAT_pet_change_XP(xp_delta);
		}			
	}
}

uint64_t last_time = 0;

void CAT_pet_tick()
{
	uint64_t now = CAT_get_RTC_now();
	// HANDLE FORWARD TIME TRAVEL BETWEEN FRAMES
	if(last_time >  0 && (now - last_time) >= CAT_MINUTE_SECONDS)
	{
		uint64_t jump = now - last_time;
		timing_state.last_stat_time += jump;
		timing_state.last_life_time += jump;
		timing_state.last_milk_time += jump;
	}
	last_time = now;
	// HANDLE BACKWARD TIME TRAVEL
	timing_state.last_stat_time = CAT_min(now, timing_state.last_stat_time);
	timing_state.last_life_time = CAT_min(now, timing_state.last_life_time);
	timing_state.last_milk_time = CAT_min(now, timing_state.last_milk_time);

	uint64_t time_since;
	uint16_t ticks;
	uint32_t remainder;

	time_since = now - timing_state.last_stat_time;
	ticks = time_since / CAT_STAT_TICK_PERIOD;
	remainder = time_since % CAT_STAT_TICK_PERIOD;
	timing_state.last_stat_time = now - remainder;
	apply_stat_ticks(ticks);

	time_since = now - timing_state.last_life_time;
	ticks = time_since / CAT_LIFE_TICK_PERIOD;
	remainder = time_since % CAT_LIFE_TICK_PERIOD;
	timing_state.last_life_time = now - remainder;
	apply_life_ticks(ticks);

	if(time_since > CAT_DAY_SECONDS)
	{
		CAT_inventory_add(ingr_time_item, 1);
	}
	
	if(CAT_pet_needs_death_report())
	{
		CAT_pet_post_death_report();
	}
}

#define WALK_COOLDOWN 4
static float walk_timer = 0;

void CAT_pet_walk()
{
	if(!is_critical() && CAT_get_battery_pct() > CAT_CRITICAL_BATTERY_PCT)
	{
		if(CAT_anim_is_in(&AM_pet, &AS_idle) && CAT_anim_is_ticking(&AM_pet))
		{	
			if(walk_timer >= WALK_COOLDOWN && CAT_room_has_free_cell())
			{
				int x, y;
				CAT_room_random_free_cell(&x, &y);
				CAT_room_cell2point(x, y, &x, &y);
				destination = (CAT_vec2) {x + 8, y + 8};

				CAT_anim_transition(&AM_pet, &AS_walk);
				walk_timer = 0;
			}
			walk_timer += CAT_get_delta_time_s();
		}
		
		if(CAT_anim_is_in(&AM_pet, &AS_walk) && CAT_anim_is_ticking(&AM_pet))
		{
			if(CAT_pet_seek(destination))
			{
				CAT_anim_transition(&AM_pet, &AS_idle);
			}
		}
	}
	else
	{
		if(!CAT_anim_is_in(&AM_pet, &AS_crit))
			CAT_anim_transition(&AM_pet, &AS_crit);
	}
}

#define REACT_DURATION 2
float react_timer = 0;

void CAT_pet_react()
{
	if(CAT_input_drag_circle(pet.pos.x, pet.pos.y-16, 16) && !CAT_anim_is_in(&AM_mood, &AS_react))
		CAT_anim_transition(&AM_mood, &AS_react);

	if(CAT_anim_is_in(&AM_mood, &AS_react))
	{
		if(react_timer >= REACT_DURATION)
		{
			uint64_t now = CAT_get_RTC_now();
			uint32_t time_since = now - timing_state.last_milk_time;

			if(time_since >= CAT_PET_MILK_COOLDOWN && timing_state.milks_produced_today < 3)
			{
				timing_state.times_milked_since_producing += 1;

				if(timing_state.times_milked_since_producing >= 5)
				{
					int x, y;
					CAT_room_random_free_cell(&x, &y);
					CAT_room_cell2point(x, y, &x, &y);

					CAT_room_spawn_pickup
					(
						(CAT_pickup)
						{
							.x0 = pet.pos.x,
							.y0 = pet.pos.y,
							.x1 = x,
							.y1 = y,
							.sprite = &milk_sprite,
							.proc = milk_proc,
							.timer = 0
						}
					);

					timing_state.milks_produced_today += 1;
					timing_state.times_milked_since_producing = 0;
				}

				timing_state.last_milk_time = now;
			}

			CAT_anim_kill(&AM_mood);
			react_timer = 0;
		}

		react_timer += CAT_get_delta_time_s();
	}
}

bool CAT_pet_is_dead()
{
	return pet.lifetime > pet.lifespan;
}

void CAT_pet_reincarnate()
{
	int incarnation_backup = pet.incarnations;
	int level_backup = pet.level;

	CAT_pet_init();

	pet.incarnations = incarnation_backup + 1;
	pet.level = level_backup;

	snprintf(pet.name, sizeof(pet.name), "%s-%d", CAT_DEFAULT_PET_NAME, pet.incarnations);

	AM_pet.state = &AS_idle;
	AM_pet.next = NULL;
	AM_pet.signal = TICK;
}

static enum
{
	NONE,
	POSTED,
	DISMISSED
} death_report_status = NONE;

void CAT_pet_post_death_report()
{
	death_report_status = POSTED;
}

void CAT_pet_dismiss_death_report()
{
	death_report_status = DISMISSED;
}

bool CAT_pet_needs_death_report()
{
	return CAT_pet_is_dead() && death_report_status == NONE;
}

bool CAT_pet_is_death_report_posted()
{
	return death_report_status == POSTED;
}