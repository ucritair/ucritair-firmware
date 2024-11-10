#include "cat_pet.h"

#include <stddef.h>
#include "cat_core.h"
#include "cat_sprite.h"
#include <stdio.h>

CAT_pet pet;

void CAT_pet_stat()
{
	pet.vigour = clamp(pet.vigour-1, 0, 12);
	pet.focus = clamp(pet.focus-1, 0, 12);
	pet.spirit = clamp(pet.spirit-1, 0, 12);
	pet.critical = !(pet.vigour > 0 && pet.focus > 0 && pet.spirit > 0);

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
	if(pet.critical)
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
	pet.vigour = 12;
	pet.focus = 12;
	pet.spirit = 12;

	pet.pos = (CAT_vec2) {120, 200};
	pet.dir = (CAT_vec2) {0, 0};
	pet.left = false;
	
	pet.stat_timer_id = CAT_timer_init(1.0f);
	pet.walk_timer_id = CAT_timer_init(4.0f);
	pet.react_timer_id = CAT_timer_init(2.0f);
	pet.action_timer_id = CAT_timer_init(2.0f);
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