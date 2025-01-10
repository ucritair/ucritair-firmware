#include "cat_pet.h"

#include <stddef.h>
#include "cat_core.h"
#include "cat_sprite.h"
#include <stdio.h>
#include <math.h>
#include "cat_item.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_gui.h"
#include "cat_bag.h"
#include <string.h>
#include "stats.h"

CAT_pet pet;

bool is_critical()
{
	return 
	(pet.vigour <= 0 || pet.focus <= 0 || pet.spirit <= 0) ||
	(CAT_get_battery_pct() < CAT_CRITICAL_BATTERY_PCT && !CAT_is_charging());
}

void CAT_pet_init()
{	
	pet.vigour = 9;
	pet.focus = 9;
	pet.spirit = 9;
	pet.lifetime = 0;
	pet.xp = 0;
	pet.level = 1;

	pet.pos = (CAT_vec2) {120, 200};
	pet.dir = (CAT_vec2) {0, 0};
	pet.left = false;

	pet.stat_timer_id = CAT_timer_init(CAT_STAT_TICK_SECS);
	pet.life_timer_id = CAT_timer_init(CAT_LIFE_TICK_SECS);
	pet.walk_timer_id = CAT_timer_init(4.0f);
	pet.react_timer_id = CAT_timer_init(1.0f);

	pet.times_pet = 0;
	pet.petting_timer_id = CAT_timer_init(CAT_PET_COOLDOWN_SECS);
	pet.times_milked = 0;

	strcpy(pet.name, "Waldo");
}

void CAT_pet_reanimate()
{
	AS_idle.enter_anim_id = NULL;
	AS_idle.tick_anim_id = &pet_idle_sprite;
	AS_idle.exit_anim_id = NULL;

	AS_walk.enter_anim_id = NULL;
	AS_walk.tick_anim_id = &pet_walk_sprite;
	AS_walk.exit_anim_id = NULL;

	AS_crit.enter_anim_id = &pet_crit_vig_in_sprite;
	AS_crit.tick_anim_id = &pet_crit_vig_sprite;
	AS_crit.exit_anim_id = &pet_crit_vig_out_sprite;

	AS_react.tick_anim_id = &mood_good_sprite;

	if(pet.vigour >= 9 || pet.focus >= 9 || pet.spirit >= 9)
	{
		if(pet.vigour >= 9)
		{
			AS_idle.tick_anim_id = &pet_idle_high_vig_sprite;
			AS_walk.tick_anim_id = &pet_walk_high_vig_sprite;
		}
		else if(pet.spirit >= 9)
		{
			AS_idle.tick_anim_id = &pet_idle_high_spi_sprite;
			AS_walk.tick_anim_id = &pet_walk_high_spi_sprite;
		}
		else
		{
			AS_idle.tick_anim_id = &pet_idle_high_foc_sprite;
			AS_walk.tick_anim_id = &pet_walk_high_foc_sprite;
		}
	}
	if(pet.vigour <= 3 || pet.focus <= 3 || pet.spirit <= 3)
	{
		if(pet.vigour <= 3)
		{
			AS_idle.tick_anim_id = &pet_idle_low_vig_sprite;
			AS_walk.tick_anim_id = &pet_walk_low_vig_sprite;	
			AS_react.tick_anim_id = &mood_low_vig_sprite;
		}
		else if(pet.spirit <= 3)
		{
			AS_idle.tick_anim_id = &pet_idle_low_spi_sprite;
			AS_walk.tick_anim_id = &pet_walk_low_spi_sprite;
			AS_react.tick_anim_id = &mood_low_spi_sprite;
		}
		else
		{
			AS_idle.tick_anim_id = &pet_idle_low_foc_sprite;
			AS_walk.tick_anim_id = &pet_walk_low_foc_sprite;
			AS_react.tick_anim_id = &mood_low_foc_sprite;
		}
	}
	if(is_critical())
	{
		if(CAT_get_battery_pct() < CAT_CRITICAL_BATTERY_PCT)
		{
			AS_crit.enter_anim_id = &pet_crit_foc_in_sprite;
			AS_crit.tick_anim_id = &icon_low_battery_pet;
			AS_crit.exit_anim_id = &pet_crit_foc_out_sprite;
		}
		else if(pet.vigour <= 0)
		{
			AS_crit.enter_anim_id = &pet_crit_vig_in_sprite;
			AS_crit.tick_anim_id = &pet_crit_vig_sprite;
			AS_crit.exit_anim_id = &pet_crit_vig_out_sprite;
		}
		else if(pet.spirit <= 0)
		{
			AS_crit.enter_anim_id = &pet_crit_spi_in_sprite;
			AS_crit.tick_anim_id = &pet_crit_spi_sprite;
			AS_crit.exit_anim_id = &pet_crit_spi_out_sprite;
		}
		else
		{
			AS_crit.enter_anim_id = &pet_crit_foc_in_sprite;
			AS_crit.tick_anim_id = &pet_crit_foc_sprite;
			AS_crit.exit_anim_id = &pet_crit_foc_out_sprite;
		}
		AS_react.tick_anim_id = &mood_bad_sprite;
	}
}

void CAT_pet_settle()
{
	if(!is_critical())
	{
		CAT_animachine_transition(&pet_asm, &AS_idle);
	}
}

bool CAT_pet_seek(CAT_vec2 targ)
{
	CAT_vec2 line = CAT_vec2_sub(targ, pet.pos);
	float dist = sqrt(CAT_vec2_mag2(line));
	float step = 48.0f * CAT_get_delta_time();
	if(dist < step)
	{
		pet.pos = targ;
		pet.dir = (CAT_vec2) {0, 0};
		return true;
	}
	else
	{
		pet.dir = CAT_vec2_mul(line, 1.0f/dist);
		pet.pos = CAT_vec2_add(pet.pos, CAT_vec2_mul(pet.dir, step));
		pet.left = pet.pos.x < targ.x;
		return false;
	}
}

void CAT_pet_reposition()
{
	if(CAT_has_free_space())
	{
		CAT_ivec2 grid_pos = CAT_nearest_free_space((CAT_ivec2){8, 6});
		CAT_ivec2 world_pos = CAT_grid2world(grid_pos);
		pet.pos = (CAT_vec2) {world_pos.x, world_pos.y};
	}
}

void CAT_pet_stat(int ticks)
{
	float goodness = CAT_AQI_aggregate();
	int delta = goodness < 0.35f ? 2 : 1;

	pet.vigour = clamp(pet.vigour - delta * ticks, 0, 12);
	pet.focus = clamp(pet.focus - delta * ticks, 0, 12);
	pet.spirit = clamp(pet.spirit - delta * ticks, 0, 12);

	bool mask = CAT_item_list_find(&bag, mask_item) != -1;
	bool pure = CAT_room_find(prop_purifier_item) != -1;
	bool uv = CAT_room_find(prop_uv_lamp_item) != -1;

	//TODO: UNFAKE THIS
	if(pet.vigour <= 0 || pet.focus <= 0 || pet.spirit <= 0)
	{
		float xp_delta = round(((float) level_cutoffs[pet.level-1]) * 0.1f);
		xp_delta *= mask ? 0.75f : 1;
		xp_delta *= pure ? 0.75f : 1;
		xp_delta *= uv ? 0.75f : 1;
		pet.xp -= xp_delta * ticks;
	}
}

void CAT_pet_life(int ticks)
{
	pet.lifetime += ticks;

	int xp = round(((pet.vigour + pet.focus + pet.spirit) / 3.0f) * 50.0f);
	pet.xp += xp * ticks;
	int cutoff = level_cutoffs[pet.level-1];
	if(pet.xp >= cutoff)
	{
		pet.level += 1;
		pet.xp -= cutoff;
	}
}

static CAT_vec2 destination = {120, 200};

void milk_proc()
{
	CAT_item_list_add(&bag, food_milk_item, 1);
}

void CAT_pet_tick(bool capture_input)
{
	if(CAT_timer_tick(pet.stat_timer_id))
	{
		CAT_pet_stat(1);
		CAT_pet_reanimate();
		CAT_timer_reset(pet.stat_timer_id);
	}

	if(CAT_timer_tick(pet.life_timer_id))
	{
		CAT_pet_life(1);
		pet.times_milked = 0;
		CAT_timer_reset(pet.life_timer_id);
	}

	CAT_timer_tick(pet.petting_timer_id);

	if(machine == CAT_MS_room)
	{
		if(!is_critical() && CAT_get_battery_pct() > CAT_CRITICAL_BATTERY_PCT)
		{
			if(CAT_animachine_is_in(&pet_asm, &AS_idle) && CAT_animachine_is_ticking(&pet_asm))
			{
				if(CAT_timer_tick(pet.walk_timer_id) && CAT_has_free_space())
				{
					CAT_ivec2 grid_dest = CAT_rand_free_space();
					CAT_ivec2 world_dest = CAT_grid2world(grid_dest);
					destination = (CAT_vec2) {world_dest.x + 8, world_dest.y + 8};

					CAT_animachine_transition(&pet_asm, &AS_walk);
					CAT_timer_reset(pet.walk_timer_id);
				}
			}
			
			if(CAT_animachine_is_in(&pet_asm, &AS_walk) && CAT_animachine_is_ticking(&pet_asm))
			{
				if(CAT_pet_seek(destination))
				{
					CAT_animachine_transition(&pet_asm, &AS_idle);
				}
			}
		}
		else
		{
			if(!CAT_animachine_is_in(&pet_asm, &AS_crit))
				CAT_animachine_transition(&pet_asm, &AS_crit);
		}
	}

	if(!capture_input)
		return;

	if(!CAT_animachine_is_in(&react_asm, &AS_react) && CAT_input_drag(pet.pos.x, pet.pos.y-16, 16))
	{
		CAT_animachine_transition(&react_asm, &AS_react);
	}
	if(CAT_animachine_is_in(&react_asm, &AS_react))
	{
		if(CAT_timer_tick(pet.react_timer_id))
		{
			CAT_animachine_transition(&react_asm, NULL);
			CAT_timer_reset(pet.react_timer_id);

			if(CAT_timer_done(pet.petting_timer_id) && pet.times_milked < 3)
			{
				pet.times_pet += 1;
				if(pet.times_pet >= 5)
				{
					CAT_ivec2 place_grid = CAT_rand_free_space();
					CAT_ivec2 place_world = CAT_grid2world(place_grid);
					CAT_vec2 place = {place_world.x, place_world.y};

					CAT_spawn_pickup
					(
						pet.pos,
						place,
						&milk_sprite,
						milk_proc
					);
					pet.times_milked += 1;
					pet.times_pet = 0;
				}
				CAT_timer_reset(pet.petting_timer_id);
			}
		}
	}
}

void CAT_render_pet(int cycle)
{
	if(cycle == 0)
	{
		int mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;
		if(pet.left)
			mode |= CAT_DRAW_MODE_REFLECT_X;
		int layer = CAT_get_battery_pct() <= CAT_CRITICAL_BATTERY_PCT ? 1 : 2;
		CAT_draw_queue_add(CAT_animachine_tick(&pet_asm), -1, layer, pet.pos.x, pet.pos.y, mode);
		if(CAT_animachine_is_in(&react_asm, &AS_react))
		{
			int x_off = pet.left ? 16 : -16;
			CAT_draw_queue_add(CAT_animachine_tick(&react_asm), -1, layer+1, pet.pos.x + x_off, pet.pos.y - 48, mode);	
		}
	}
}