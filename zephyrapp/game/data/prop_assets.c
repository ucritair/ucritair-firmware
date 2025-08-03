#include "prop_assets.h"
#include "sprite_assets.h"
#include "cat_procs.h"

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
		(int16_t[]) {0,16,31,31,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
		{
			.aabb = {0,16,31,31,},
			.tx = 0,
			.ty = 1,
			.proc = interact_proc_reed,
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
const CAT_prop reed_house_prop = {
	.sprite = &reed_house_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,40,63,79,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop statue_prop = {
	.sprite = &reed_house_sprite,
	.blockers = (int16_t*[]) {
	},
	.blocker_count = 0,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
