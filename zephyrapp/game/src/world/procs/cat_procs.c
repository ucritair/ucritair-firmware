#include "cat_procs.h"

#include "cat_item.h"
#include "item_assets.h"
#include "cat_dialogue.h"
#include "dialogue_assets.h"
#include "cat_math.h"

void interact_proc_reed()
{
	CAT_enter_dialogue(&dialogue_reed_default);
}

void interact_proc_reed_house()
{
	CAT_enter_dialogue(&dialogue_reed_house);
}

void interact_proc_statue()
{
	if(CAT_rand_chance(32))
		CAT_enter_dialogue(&dialogue_statue_chirp);
	else
		CAT_enter_dialogue(&dialogue_statue_default);
}