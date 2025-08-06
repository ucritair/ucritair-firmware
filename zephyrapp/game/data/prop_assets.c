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
	.sprite = &world_reed_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,16,32,32,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
		{
			.aabb = {16,16,32,32,},
			.tx = 0,
			.ty = 1,
			.proc = interact_proc_reed,
		},
	},
	.trigger_count = 1,
};
const CAT_prop reed_house_prop = {
	.sprite = &world_reed_house_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,56,63,80,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop statue_prop = {
	.sprite = &world_statue_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {16,32,32,40,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop bldg_sm_prop = {
	.sprite = &world_bldg_sm_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,56,32,80,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop bldg_md_prop = {
	.sprite = &world_bldg_md_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,56,40,80,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop bldg_lg_prop = {
	.sprite = &world_bldg_lg_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,56,48,80,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
