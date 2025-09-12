#include "prop_assets.h"
#include "sprite_assets.h"
#include "cat_procs.h"

const CAT_prop null_prop = {
	.sprite = &null_sprite,
	.blockers = (int8_t*[]) {
	},
	.blocker_count = 0,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop reed_prop = {
	.sprite = &world_reed_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,1,2,2,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
		{
			.aabb = {0,1,2,2,},
			.tx = 0,
			.ty = 1,
			.proc = interact_proc_reed,
		},
	},
	.trigger_count = 1,
};
const CAT_prop reed_house_prop = {
	.sprite = &world_reed_house_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,3,4,5,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
		{
			.aabb = {0,4,2,5,},
			.tx = 0,
			.ty = 1,
			.proc = interact_proc_reed_house,
		},
	},
	.trigger_count = 1,
};
const CAT_prop statue_prop = {
	.sprite = &world_statue_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {1,2,2,3,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
		{
			.aabb = {1,2,2,3,},
			.tx = 0,
			.ty = 1,
			.proc = interact_proc_statue,
		},
	},
	.trigger_count = 1,
};
const CAT_prop bldg_sm_prop = {
	.sprite = &world_bldg_sm_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,3,2,5,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop bldg_md_prop = {
	.sprite = &world_bldg_md_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,3,3,5,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop bldg_lg_prop = {
	.sprite = &world_bldg_lg_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,3,3,5,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop arcade_prop = {
	.sprite = &world_arcade_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,3,6,5,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
		{
			.aabb = {0,4,2,5,},
			.tx = 0,
			.ty = 1,
			.proc = interact_proc_arcade,
		},
	},
	.trigger_count = 1,
};
const CAT_prop market_prop = {
	.sprite = &world_market_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,2,3,4,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
		{
			.aabb = {1,3,3,4,},
			.tx = 0,
			.ty = 1,
			.proc = interact_proc_market,
		},
	},
	.trigger_count = 1,
};
const CAT_prop depot_prop = {
	.sprite = &world_depot_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,2,3,4,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop dumpster_prop = {
	.sprite = &world_dumpster_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,1,1,2,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop trash_prop = {
	.sprite = &world_trash_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,0,1,1,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop bench_prop = {
	.sprite = &world_bench_tile_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,0,1,1,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop track_prop = {
	.sprite = &floor_grass_tile_sprite,
	.blockers = (int8_t*[]) {
	},
	.blocker_count = 0,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
const CAT_prop fence_prop = {
	.sprite = &floor_grass_tile_sprite,
	.blockers = (int8_t*[]) {
		(int8_t[]) {0,0,16,16,},
	},
	.blocker_count = 1,
	.triggers = (struct trigger[]) {
	},
	.trigger_count = 0,
};
