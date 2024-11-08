#include "cat_pet.h"

CAT_pet pet;

CAT_ASM_state* pet_asm;
CAT_ASM_state AS_idle;
CAT_ASM_state AS_walk;
CAT_ASM_state AS_adjust_in;
CAT_ASM_state AS_walk_action;
CAT_ASM_state AS_eat;
CAT_ASM_state AS_study;
CAT_ASM_state AS_play;
CAT_ASM_state AS_adjust_out;
CAT_ASM_state AS_vig_up;
CAT_ASM_state AS_foc_up;
CAT_ASM_state AS_spi_up;

CAT_ASM_state* bubble_asm;
CAT_ASM_state AS_react;

void CAT_pet_anim_init()
{
	CAT_ASM_init(&AS_idle, -1, pet_idle_high_vig_sprite, -1);
	CAT_ASM_init(&AS_walk, -1, pet_walk_high_vig_sprite, -1);
	CAT_ASM_init(&AS_adjust_in, -1, -1, pet_wings_in_sprite);
	CAT_ASM_init(&AS_walk_action, -1, pet_walk_sprite, -1);
	CAT_ASM_init(&AS_eat, pet_eat_down_sprite, pet_chew_sprite, pet_eat_up_sprite);
	CAT_ASM_init(&AS_study, pet_eat_down_sprite, pet_chew_sprite, pet_eat_up_sprite);
	CAT_ASM_init(&AS_play, pet_eat_down_sprite, pet_chew_sprite, pet_eat_up_sprite);
	CAT_ASM_init(&AS_adjust_out, -1, -1, pet_wings_out_sprite);
	CAT_ASM_init(&AS_vig_up, -1, -1, pet_vig_up_sprite);
	CAT_ASM_init(&AS_foc_up, -1, -1, pet_foc_up_sprite);
	CAT_ASM_init(&AS_spi_up, -1, -1, pet_spi_up_sprite);

	CAT_ASM_init(&AS_react, -1, bubl_react_good_sprite, -1);
}

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

	CAT_pet_anim_init();
}