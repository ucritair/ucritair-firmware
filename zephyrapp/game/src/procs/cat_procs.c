#include "cat_procs.h"

#include "cat_item.h"
#include "item_assets.h"
#include "cat_dialogue.h"
#include "dialogue_assets.h"
#include "cat_math.h"
#include "cat_machine.h"
#include "cat_room.h"
#include "dialogue_profile_assets.h"

void interact_proc_reed()
{
	CAT_activate_dialogue_profile(&dialogue_profile_reed);
	CAT_enter_dialogue(CAT_poll_dialogue_profile());
}

void interact_proc_reed_house()
{
	CAT_enter_dialogue(&dialogue_reed_house);
}

void proc_coc_innerworld()
{
	CAT_machine_transition(CAT_MS_room);
}

void interact_proc_statue()
{
	CAT_activate_dialogue_profile(&dialogue_profile_bird_statue);
	CAT_enter_dialogue(CAT_poll_dialogue_profile());
}