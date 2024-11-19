#include "cat_pet.h"

#include <stddef.h>
#include "cat_core.h"
#include "cat_sprite.h"
#include <stdio.h>
#include <math.h>
#include "cat_item.h"
#include "cat_room.h"
#include "cat_input.h"

CAT_pet pet =
{
	.vigour = 9,
	.focus = 9,
	.spirit = 9,
	.lifetime = 0,

	.pos = {120, 200},
	.dir = {0, 0},
	.left = false,

	.stat_timer_id = -1,
	.walk_timer_id = -1,
	.react_timer_id = -1
};
CAT_vec2 destination = {120, 200};

void CAT_pet_stat(int ticks)
{
	int mask = CAT_gear_status(mask_item);
	int pure = CAT_room_find(purifier_item) != -1;
	int uv = CAT_room_find(uv_item) != -1;

	float dv_aq =
	(
		CAT_pm25_score(aqi.sen5x.pm2_5) +
		CAT_nox_score(aqi.sen5x.nox_index) +
		CAT_voc_score(aqi.sen5x.voc_index)
	) * 0.25f;
	float df_aq =
	(
		CAT_co2_score(aqi.sunrise.ppm_filtered_compensated) +
		CAT_nox_score(aqi.sen5x.nox_index)
	) * 0.33f;
	float ds_aq =
	(
		CAT_co2_score(aqi.sunrise.ppm_filtered_compensated) +
		CAT_temperature_score(CAT_mean_temp())
	) * 0.33f;
	if(dv_aq >= 1)
		dv_aq -= pure;
	if(df_aq >= 1)
		df_aq -= uv;
	if(ds_aq >= 1)
		ds_aq -= mask;
	
	int dv = round(1 + dv_aq);
	int df = round(1 + df_aq);
	int ds = round(1 + ds_aq);

	pet.vigour = clamp(pet.vigour - dv * ticks, 0, 12);
	pet.focus = clamp(pet.focus - df * ticks, 0, 12);
	pet.spirit = clamp(pet.spirit - ds * ticks, 0, 12);
}

void CAT_pet_life(int ticks)
{
	pet.lifetime += ticks;
}

void CAT_pet_use(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return;
	
	pet.vigour = clamp(pet.vigour + item->data.tool_data.dv, 0, 12);
	pet.focus = clamp(pet.focus + item->data.tool_data.df, 0, 12);
	pet.spirit = clamp(pet.spirit + item->data.tool_data.ds, 0, 12);
}

bool CAT_pet_is_critical()
{
	return !(pet.vigour > 0 && pet.focus > 0 && pet.spirit > 0);
}

void CAT_pet_reanimate()
{
	AS_idle.enter_anim_id = -1;
	AS_idle.tick_anim_id = pet_idle_sprite;
	AS_idle.exit_anim_id = -1;

	AS_walk.enter_anim_id = -1;
	AS_walk.tick_anim_id = pet_walk_sprite;
	AS_walk.exit_anim_id = -1;

	AS_crit.enter_anim_id = pet_crit_vig_in_sprite;
	AS_crit.tick_anim_id = pet_crit_vig_sprite;
	AS_crit.exit_anim_id = pet_crit_vig_out_sprite;

	AS_adjust_in.exit_anim_id = pet_idle_sprite;
	AS_adjust_out.exit_anim_id = pet_idle_sprite;

	AS_react.tick_anim_id = mood_good_sprite;

	if(pet.vigour >= 9 || pet.focus >= 9 || pet.spirit >= 9)
	{
		if(pet.vigour >= 9)
		{
			AS_idle.tick_anim_id = pet_idle_high_vig_sprite;
			AS_walk.tick_anim_id = pet_walk_high_vig_sprite;
		}
		else if(pet.spirit >= 9)
		{
			AS_idle.tick_anim_id = pet_idle_high_spi_sprite;
			AS_walk.tick_anim_id = pet_walk_high_spi_sprite;
		}
		else
		{
			AS_idle.tick_anim_id = pet_idle_high_foc_sprite;
			AS_walk.tick_anim_id = pet_walk_high_foc_sprite;
		}
	}
	if(pet.vigour <= 3 || pet.focus <= 3 || pet.spirit <= 3)
	{
		if(pet.vigour <= 3)
		{
			AS_idle.tick_anim_id = pet_idle_low_vig_sprite;
			AS_walk.tick_anim_id = pet_walk_low_vig_sprite;	
			AS_react.tick_anim_id = mood_low_vig_sprite;
		}
		else if(pet.spirit <= 3)
		{
			AS_idle.tick_anim_id = pet_idle_low_spi_sprite;
			AS_walk.tick_anim_id = pet_walk_low_spi_sprite;
			AS_react.tick_anim_id = mood_low_spi_sprite;
		}
		else
		{
			AS_idle.tick_anim_id = pet_idle_low_foc_sprite;
			AS_walk.tick_anim_id = pet_walk_low_foc_sprite;
			AS_react.tick_anim_id = mood_low_foc_sprite;
		}
	}
	if(CAT_pet_is_critical())
	{
		if(pet.vigour <= 0)
		{
			AS_crit.enter_anim_id = pet_crit_vig_in_sprite;
			AS_crit.tick_anim_id = pet_crit_vig_sprite;
			AS_crit.exit_anim_id = pet_crit_vig_out_sprite;
		}
		else if(pet.spirit <= 0)
		{
			AS_crit.enter_anim_id = pet_crit_spi_in_sprite;
			AS_crit.tick_anim_id = pet_crit_spi_sprite;
			AS_crit.exit_anim_id = pet_crit_spi_out_sprite;
		}
		else
		{
			AS_crit.enter_anim_id = pet_crit_foc_in_sprite;
			AS_crit.tick_anim_id = pet_crit_foc_sprite;
			AS_crit.exit_anim_id = pet_crit_foc_out_sprite;
		}
		AS_react.tick_anim_id = mood_bad_sprite;
	}
}

void CAT_pet_settle()
{
	if(!CAT_pet_is_critical())
	{
		CAT_AM_transition(&pet_asm, &AS_idle);
	}
}

bool CAT_pet_seek(CAT_vec2 targ)
{
	CAT_vec2 line = CAT_vec2_sub(targ, pet.pos);
	float dist = CAT_vec2_mag(line);
	float step = 48.0f * CAT_get_delta_time();
	if(dist < step)
	{
		pet.pos = targ;
		pet.dir = (CAT_vec2) {0, 0};
		return true;
	}
	else
	{
		pet.dir = CAT_vec2_div(line, dist);
		pet.pos = CAT_vec2_add(pet.pos, CAT_vec2_mul(pet.dir, step));
		pet.left = pet.pos.x < targ.x;
		return false;
	}
}

void CAT_pet_init()
{	
	pet.stat_timer_id = CAT_timer_init(CAT_STAT_TICK_SECS);
	pet.life_timer_id = CAT_timer_init(CAT_LIFE_TICK_SECS);
	pet.walk_timer_id = CAT_timer_init(4.0f);
	pet.react_timer_id = CAT_timer_init(1.0f);
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
		CAT_timer_reset(pet.life_timer_id);
	}

	if(machine == CAT_MS_room)
	{
		if(!CAT_pet_is_critical())
		{
			if(CAT_AM_is_in(&pet_asm, &AS_idle) && CAT_AM_is_ticking(&pet_asm))
			{
				if(CAT_timer_tick(pet.walk_timer_id))
				{
					CAT_ivec2 grid_min = CAT_ivec2_add(room.bounds.min, (CAT_ivec2){1, 1});
					CAT_ivec2 grid_max = CAT_ivec2_add(room.bounds.max, (CAT_ivec2){-1, -1});
					CAT_vec2 world_min = CAT_iv2v(CAT_ivec2_mul(grid_min, 16));
					CAT_vec2 world_max = CAT_iv2v(CAT_ivec2_mul(grid_max, 16));
					destination =
					(CAT_vec2) {
						CAT_rand_float(world_min.x, world_max.x),
						CAT_rand_float(world_min.y, world_max.y)
					};

					CAT_AM_transition(&pet_asm, &AS_walk);
					CAT_timer_reset(pet.walk_timer_id);
				}
			}
			
			if(CAT_AM_is_in(&pet_asm, &AS_walk) && CAT_AM_is_ticking(&pet_asm))
			{
				if(CAT_pet_seek(destination))
				{
					CAT_AM_transition(&pet_asm, &AS_idle);
				}
			}
		}
		else
		{
			if(!CAT_AM_is_in(&pet_asm, &AS_crit))
				CAT_AM_transition(&pet_asm, &AS_crit);
		}
	}

	if(!capture_input)
		return;

	if(!CAT_AM_is_in(&react_asm, &AS_react) && CAT_input_drag(pet.pos.x, pet.pos.y-16, 16))
	{
		CAT_AM_transition(&react_asm, &AS_react);
	}
	if(CAT_AM_is_in(&react_asm, &AS_react))
	{
		if(CAT_timer_tick(pet.react_timer_id))
		{
			CAT_AM_transition(&react_asm, NULL);
			CAT_timer_reset(pet.react_timer_id);
		}
	}
}

void CAT_render_pet(int cycle)
{
	if(cycle == 0)
	{
		int pet_mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;
		if(pet.left)
			pet_mode |= CAT_DRAW_MODE_REFLECT_X;
		CAT_draw_queue_animate(CAT_AM_tick(&pet_asm), 2, pet.pos.x, pet.pos.y, pet_mode);	
		if(react_asm != NULL)
		{
			int x_off = pet.left ? 16 : -16;
			CAT_draw_queue_animate(CAT_AM_tick(&react_asm), 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);	
		}
	}
}