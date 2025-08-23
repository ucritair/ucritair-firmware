#include "scene_assets.h"
#include "prop_assets.h"
#include "cat_render.h"
#include "sprite_assets.h"

const CAT_scene test_scene = {
	.bounds = {-24, -24, 24, 24},
	.background = {
		.colour = 36144,
		.palette = &floor_grass_tile_sprite,
		.tiles = (struct tile[]) {
			{
				.x = -11,
				.y = 0,
				.frame = 0,
			},
			{
				.x = -11,
				.y = 1,
				.frame = 6,
			},
			{
				.x = -10,
				.y = 0,
				.frame = 13,
			},
			{
				.x = -9,
				.y = 0,
				.frame = 13,
			},
			{
				.x = -8,
				.y = 0,
				.frame = 13,
			},
			{
				.x = -7,
				.y = 0,
				.frame = 13,
			},
			{
				.x = -6,
				.y = 0,
				.frame = 13,
			},
			{
				.x = -5,
				.y = 0,
				.frame = 13,
			},
			{
				.x = -4,
				.y = 0,
				.frame = 13,
			},
			{
				.x = -3,
				.y = 0,
				.frame = 13,
			},
			{
				.x = -2,
				.y = 0,
				.frame = 13,
			},
			{
				.x = -11,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -9,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -10,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -8,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -7,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -6,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -5,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -3,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -4,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -2,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -1,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -1,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 0,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 1,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 1,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 2,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 3,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 3,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 4,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 5,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 6,
				.y = -10,
				.frame = 13,
			},
			{
				.x = -1,
				.y = 0,
				.frame = 13,
			},
			{
				.x = 1,
				.y = 0,
				.frame = 13,
			},
			{
				.x = 0,
				.y = 0,
				.frame = 13,
			},
			{
				.x = 2,
				.y = 0,
				.frame = 13,
			},
			{
				.x = 3,
				.y = 0,
				.frame = 13,
			},
			{
				.x = 3,
				.y = 0,
				.frame = 13,
			},
			{
				.x = 4,
				.y = 0,
				.frame = 13,
			},
			{
				.x = 5,
				.y = 0,
				.frame = 13,
			},
			{
				.x = 6,
				.y = 0,
				.frame = 13,
			},
			{
				.x = -11,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -10,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -9,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -8,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -7,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -6,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -5,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -4,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -3,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -2,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -1,
				.y = -9,
				.frame = 15,
			},
			{
				.x = 0,
				.y = -9,
				.frame = 15,
			},
			{
				.x = 1,
				.y = -9,
				.frame = 15,
			},
			{
				.x = 2,
				.y = -9,
				.frame = 15,
			},
			{
				.x = 3,
				.y = -9,
				.frame = 15,
			},
			{
				.x = 4,
				.y = -9,
				.frame = 15,
			},
			{
				.x = 5,
				.y = -9,
				.frame = 15,
			},
			{
				.x = 6,
				.y = -9,
				.frame = 15,
			},
			{
				.x = -10,
				.y = 1,
				.frame = 15,
			},
			{
				.x = -9,
				.y = 1,
				.frame = 15,
			},
			{
				.x = -8,
				.y = 1,
				.frame = 15,
			},
			{
				.x = -7,
				.y = 1,
				.frame = 15,
			},
			{
				.x = -6,
				.y = 1,
				.frame = 15,
			},
			{
				.x = -5,
				.y = 1,
				.frame = 15,
			},
			{
				.x = -4,
				.y = 1,
				.frame = 15,
			},
			{
				.x = -3,
				.y = 1,
				.frame = 15,
			},
			{
				.x = -2,
				.y = 1,
				.frame = 15,
			},
			{
				.x = -1,
				.y = 1,
				.frame = 15,
			},
			{
				.x = 0,
				.y = 1,
				.frame = 15,
			},
			{
				.x = 1,
				.y = 1,
				.frame = 15,
			},
			{
				.x = 2,
				.y = 1,
				.frame = 15,
			},
			{
				.x = 3,
				.y = 1,
				.frame = 15,
			},
			{
				.x = 4,
				.y = 1,
				.frame = 15,
			},
			{
				.x = 4,
				.y = 1,
				.frame = 15,
			},
			{
				.x = 5,
				.y = 1,
				.frame = 15,
			},
			{
				.x = 6,
				.y = 1,
				.frame = 15,
			},
			{
				.x = -12,
				.y = -10,
				.frame = 0,
			},
			{
				.x = -12,
				.y = -9,
				.frame = 6,
			},
			{
				.x = 7,
				.y = -10,
				.frame = 2,
			},
			{
				.x = 7,
				.y = -9,
				.frame = 12,
			},
			{
				.x = 7,
				.y = -8,
				.frame = 14,
			},
			{
				.x = 7,
				.y = -7,
				.frame = 14,
			},
			{
				.x = 7,
				.y = -6,
				.frame = 14,
			},
			{
				.x = 7,
				.y = -5,
				.frame = 14,
			},
			{
				.x = 7,
				.y = -4,
				.frame = 14,
			},
			{
				.x = 7,
				.y = -3,
				.frame = 14,
			},
			{
				.x = 7,
				.y = -2,
				.frame = 14,
			},
			{
				.x = 7,
				.y = -1,
				.frame = 14,
			},
			{
				.x = 7,
				.y = 0,
				.frame = 10,
			},
			{
				.x = 7,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 7,
				.y = -10,
				.frame = 9,
			},
			{
				.x = 7,
				.y = -10,
				.frame = 13,
			},
			{
				.x = 8,
				.y = -10,
				.frame = 2,
			},
			{
				.x = 8,
				.y = -9,
				.frame = 16,
			},
			{
				.x = 8,
				.y = -7,
				.frame = 16,
			},
			{
				.x = 8,
				.y = -8,
				.frame = 16,
			},
			{
				.x = 8,
				.y = -6,
				.frame = 16,
			},
			{
				.x = 8,
				.y = -5,
				.frame = 16,
			},
			{
				.x = 8,
				.y = -4,
				.frame = 16,
			},
			{
				.x = 8,
				.y = -3,
				.frame = 16,
			},
			{
				.x = 8,
				.y = -2,
				.frame = 16,
			},
			{
				.x = 8,
				.y = -1,
				.frame = 16,
			},
			{
				.x = 8,
				.y = 0,
				.frame = 16,
			},
			{
				.x = 8,
				.y = 1,
				.frame = 8,
			},
			{
				.x = 7,
				.y = 1,
				.frame = 15,
			},
			{
				.x = 2,
				.y = -1,
				.frame = 10,
			},
			{
				.x = 2,
				.y = 0,
				.frame = 10,
			},
			{
				.x = 3,
				.y = 0,
				.frame = 9,
			},
			{
				.x = 2,
				.y = -3,
				.frame = 0,
			},
			{
				.x = 3,
				.y = -3,
				.frame = 2,
			},
			{
				.x = 3,
				.y = -2,
				.frame = 16,
			},
			{
				.x = 3,
				.y = -1,
				.frame = 16,
			},
			{
				.x = 2,
				.y = -2,
				.frame = 14,
			},
			{
				.x = 2,
				.y = -1,
				.frame = 14,
			},
			{
				.x = 0,
				.y = 1,
				.frame = 11,
			},
			{
				.x = -1,
				.y = 1,
				.frame = 12,
			},
			{
				.x = 0,
				.y = 2,
				.frame = 16,
			},
			{
				.x = 0,
				.y = 3,
				.frame = 16,
			},
			{
				.x = 0,
				.y = 4,
				.frame = 16,
			},
			{
				.x = 0,
				.y = 5,
				.frame = 16,
			},
			{
				.x = 0,
				.y = 6,
				.frame = 16,
			},
			{
				.x = 0,
				.y = 7,
				.frame = 16,
			},
			{
				.x = 0,
				.y = 8,
				.frame = 16,
			},
			{
				.x = -1,
				.y = 9,
				.frame = 6,
			},
			{
				.x = 0,
				.y = 9,
				.frame = 8,
			},
			{
				.x = -1,
				.y = 2,
				.frame = 14,
			},
			{
				.x = -1,
				.y = 3,
				.frame = 14,
			},
			{
				.x = -1,
				.y = 4,
				.frame = 14,
			},
			{
				.x = -1,
				.y = 5,
				.frame = 14,
			},
			{
				.x = -1,
				.y = 6,
				.frame = 14,
			},
			{
				.x = -1,
				.y = 7,
				.frame = 14,
			},
			{
				.x = -1,
				.y = 8,
				.frame = 14,
			},
		},
		.tile_count = 133,
	},
	.layers = (struct layer[]) {
		{
			.props = (struct prop[]) {
				{
					.prop = &bldg_sm_prop,
					.position_x = 7,
					.position_y = -18,
					.variant = -1,
				},
				{
					.prop = &bldg_lg_prop,
					.position_x = -4,
					.position_y = -17,
					.variant = -1,
				},
				{
					.prop = &bldg_sm_prop,
					.position_x = -1,
					.position_y = -17,
					.variant = -1,
				},
				{
					.prop = &bldg_lg_prop,
					.position_x = 4,
					.position_y = -16,
					.variant = -1,
				},
				{
					.prop = &bldg_md_prop,
					.position_x = -7,
					.position_y = -16,
					.variant = -1,
				},
				{
					.prop = &bldg_sm_prop,
					.position_x = -9,
					.position_y = -16,
					.variant = -1,
				},
				{
					.prop = &bldg_md_prop,
					.position_x = 1,
					.position_y = -16,
					.variant = -1,
				},
				{
					.prop = &market_prop,
					.position_x = -4,
					.position_y = -14,
					.variant = -1,
				},
				{
					.prop = &depot_prop,
					.position_x = -1,
					.position_y = -14,
					.variant = -1,
				},
				{
					.prop = &bldg_lg_prop,
					.position_x = -12,
					.position_y = -15,
					.variant = -1,
				},
				{
					.prop = &reed_prop,
					.position_x = -8,
					.position_y = -12,
					.variant = -1,
				},
				{
					.prop = &bldg_md_prop,
					.position_x = -10,
					.position_y = -8,
					.variant = -1,
				},
				{
					.prop = &bldg_sm_prop,
					.position_x = -12,
					.position_y = -8,
					.variant = -1,
				},
				{
					.prop = &reed_house_prop,
					.position_x = 2.0,
					.position_y = -8.0,
					.variant = -1,
				},
				{
					.prop = &bldg_lg_prop,
					.position_x = -7,
					.position_y = -7,
					.variant = -1,
				},
				{
					.prop = &arcade_prop,
					.position_x = -4,
					.position_y = -7,
					.variant = -1,
				},
				{
					.prop = &bench_prop,
					.position_x = -10.0,
					.position_y = -1.0,
					.variant = 0,
				},
				{
					.prop = &bench_prop,
					.position_x = -9.0,
					.position_y = -1.0,
					.variant = 1,
				},
				{
					.prop = &bench_prop,
					.position_x = -7.0,
					.position_y = -1.0,
					.variant = 0,
				},
				{
					.prop = &bench_prop,
					.position_x = -6.0,
					.position_y = -1.0,
					.variant = 1,
				},
				{
					.prop = &statue_prop,
					.position_x = -13,
					.position_y = -1,
					.variant = -1,
				},
				{
					.prop = &bench_prop,
					.position_x = -10.0,
					.position_y = 2.0,
					.variant = 2,
				},
				{
					.prop = &bench_prop,
					.position_x = -9,
					.position_y = 2,
					.variant = 3,
				},
				{
					.prop = &bench_prop,
					.position_x = -7.0,
					.position_y = 2.0,
					.variant = 2,
				},
				{
					.prop = &bench_prop,
					.position_x = -6.0,
					.position_y = 2.0,
					.variant = 3,
				},
			},
			.prop_count = 25,
		},
	},
	.layer_count = 1,
};
