#include "dialogue_profile_assets.h"
#include "dialogue_assets.h"
#include "cat_procs.h"

const CAT_dialogue_profile dialogue_profile_reed = {
	.entries = (CAT_dialogue_profile_entry[]) {
		(CAT_dialogue_profile_entry) {
			.node = &dialogue_reed_default,
			.is_active_proc = NULL,
			.weight = 0,
		},
		(CAT_dialogue_profile_entry) {
			.node = &dialogue_reed_question_fish,
			.is_active_proc = NULL,
			.weight = 0,
		},
	},
	.entry_count = 2,
	
	.mandatory_node = NULL,
	.opener_probability = 0.0,
};
const CAT_dialogue_profile dialogue_profile_bird_statue = {
	.entries = (CAT_dialogue_profile_entry[]) {
		(CAT_dialogue_profile_entry) {
			.node = &dialogue_statue_default,
			.is_active_proc = NULL,
			.weight = 1,
		},
		(CAT_dialogue_profile_entry) {
			.node = &dialogue_statue_chirp,
			.is_active_proc = NULL,
			.weight = 32,
		},
	},
	.entry_count = 2,
	
	.mandatory_node = NULL,
	.opener_probability = 0.0,
};
