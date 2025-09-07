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

void interact_proc_statue()
{
	CAT_activate_dialogue_profile(&dialogue_profile_bird_statue);
	CAT_enter_dialogue(CAT_poll_dialogue_profile());
}

void interact_proc_market()
{
	CAT_enter_dialogue(&dialogue_market_default);
}

void interact_proc_arcade()
{
	CAT_enter_dialogue(&dialogue_arcade_default);
}

void proc_coc_innerworld()
{
	CAT_pushdown_transition(CAT_MS_room);
}

