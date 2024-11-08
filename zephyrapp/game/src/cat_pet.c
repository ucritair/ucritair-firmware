#include <stddef.h>
#include "cat_pet.h"
#include "cat_core.h"
#include "cat_sprite.h"

CAT_pet pet;

void CAT_pet_stat()
{
	float dv = -1;
	float df = -1;
	float ds = -1;

	pet.vigour += dv;
	pet.focus += df;
	pet.spirit += ds;
	pet.critical = (pet.vigour >= 1 && pet.focus >= 1 && pet.spirit >= 1);
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
	
	pet.stat_timer_id = CAT_timer_init(5.0f);
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
		if(bubble_asm != NULL)
		{
			int x_off = pet.left ? 16 : -16;
			CAT_draw_queue_animate(CAT_AM_tick(&bubble_asm), 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);	
		}
	}
}