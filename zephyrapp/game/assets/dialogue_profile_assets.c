#include "dialogue_assets.h"
#include "cat_procs.h"

const CAT_dialogue_profile dialogue_profile_reed =
{
	.entries = (struct dialogue_profile_entry[])
	{
		{
			.node = &dialogue_reed_default,
			.weight = 0,
		},
		{
			.node = &dialogue_reed_question_fish,
			.weight = 0,
		},
	},
	.entry_count = 2,
};
const CAT_dialogue_profile dialogue_profile_bird_statue =
{
	.entries = (struct dialogue_profile_entry[])
	{
		{
			.node = &dialogue_statue_chirp,
			.weight = 1,
		},
		{
			.node = &dialogue_statue_default,
			.weight = 32,
		},
	},
	.entry_count = 2,
};
