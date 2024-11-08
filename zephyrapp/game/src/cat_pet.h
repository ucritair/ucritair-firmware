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

void CAT_pet_stat();
bool CAT_pet_seek(CAT_vec2 targ);
void CAT_pet_init();
void CAT_render_pet(int cycle);

#endif