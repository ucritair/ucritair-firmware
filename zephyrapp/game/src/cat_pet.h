#ifndef CAT_PET_H
#define CAT_PET_H

#include "cat_machine.h"
#include "cat_math.h"

typedef struct CAT_pet
{
	float vigour;
	float focus;
	float spirit;
	bool critical;

	CAT_vec2 pos;
	CAT_vec2 dir;
	bool left;
	
	int stat_timer_id;
	int walk_timer_id;
	int react_timer_id;
	int action_timer_id;
} CAT_pet;
extern CAT_pet pet;

extern CAT_ASM_state* pet_asm;
extern CAT_ASM_state AS_idle;
extern CAT_ASM_state AS_walk;
extern CAT_ASM_state AS_adjust_in;
extern CAT_ASM_state AS_walk_action;
extern CAT_ASM_state AS_eat;
extern CAT_ASM_state AS_study;
extern CAT_ASM_state AS_play;
extern CAT_ASM_state AS_adjust_out;
extern CAT_ASM_state AS_vig_up;
extern CAT_ASM_state AS_foc_up;
extern CAT_ASM_state AS_spi_up;

extern CAT_ASM_state* bubble_asm;
extern CAT_ASM_state AS_react;

void CAT_pet_anim_init();
void CAT_pet_stat();
bool CAT_pet_seek(CAT_vec2 targ);
void CAT_pet_init();

#endif