#include "scene_assets.h"
#include "prop_assets.h"

const CAT_scene test_scene = {
	.layers = (struct layer[]) {
		{
			.props = (struct prop[]) {
				{
					.prop = &reed_prop,
					.position_x = 29,
					.position_y = -86,
				},
				{
					.prop = &reed_house_prop,
					.position_x = -37,
					.position_y = -160,
				},
			},
			.prop_count = 2,
		},
	},
	.layer_count = 1,
};
