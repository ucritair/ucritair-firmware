#include "prop_assets.h"
#include "sprite_assets.h"

const CAT_prop null_prop = {
	.sprite = &null_sprite,
	.blockers = (int16_t*[]) {
	},
	.blocker_count = 0,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop reed_prop = {
	.sprite = &npc_reed_sprite,
	.blockers = (int16_t*[]) {
	},
	.blocker_count = 0,
	.triggers = (struct trigger[]) {
		{
			.aabb = {0,16,31,31,},
			.proc = NULL,
		},
	},
	.trigger_count = 1,
};
const CAT_prop clouds_prop = {
	.sprite = &monitor_clouds_sprite,
	.blockers = (int16_t*[]) {
	},
	.blocker_count = 0,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
