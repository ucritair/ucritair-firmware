#include "scene_assets.h"
#include "prop_assets.h"

const CAT_scene test_scene = {
	.layers = (struct layer[]) {
		{
			.props = (struct prop[]) {
				{
					.prop = &bldg_lg_prop,
					.position_x = -120,
					.position_y = -160,
				},
				{
					.prop = &bldg_md_prop,
					.position_x = -72,
					.position_y = -160,
				},
				{
					.prop = &bldg_sm_prop,
					.position_x = -32,
					.position_y = -160,
				},
				{
					.prop = &reed_house_prop,
					.position_x = 0,
					.position_y = -160,
				},
				{
					.prop = &reed_prop,
					.position_x = 64,
					.position_y = -112,
				},
				{
					.prop = &statue_prop,
					.position_x = -24,
					.position_y = -40,
				},
			},
			.prop_count = 6,
		},
	},
	.layer_count = 1,
};
