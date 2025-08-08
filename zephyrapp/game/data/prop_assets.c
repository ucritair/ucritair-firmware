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
			.aabb = {0,16,32,32,},
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
		(int16_t[]) {0,48,64,80,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
		{
			.aabb = {0,64,32,80,},
			.tx = 0,
			.ty = 1,
			.proc = interact_proc_reed_house,
		},
	},
	.trigger_count = 1,
};
const CAT_prop statue_prop = {
	.sprite = &world_statue_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {16,32,32,48,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
		{
			.aabb = {16,32,32,48,},
			.tx = 1,
			.ty = 0,
			.proc = interact_proc_statue,
		},
	},
	.trigger_count = 1,
};
const CAT_prop bldg_sm_prop = {
	.sprite = &world_bldg_sm_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,48,32,80,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop bldg_md_prop = {
	.sprite = &world_bldg_md_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,48,48,80,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop bldg_lg_prop = {
	.sprite = &world_bldg_lg_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,48,48,80,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop arcade_prop = {
	.sprite = &world_arcade_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,48,96,80,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop market_prop = {
	.sprite = &world_market_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,32,48,64,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop depot_prop = {
	.sprite = &world_depot_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,32,48,64,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop dumpster_prop = {
	.sprite = &world_dumpster_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,16,16,32,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop trash_prop = {
	.sprite = &world_trash_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,0,16,16,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop bench_prop = {
	.sprite = &world_bench_tile_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,0,16,16,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop track_prop = {
	.sprite = &floor_grass_tile_sprite,
	.blockers = (int16_t*[]) {
	},
	.blocker_count = 0,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop fence_prop = {
	.sprite = &floor_grass_tile_sprite,
	.blockers = (int16_t*[]) {
		(int16_t[]) {0,0,16,16,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
