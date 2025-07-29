#include "scene_assets.h"
#include "prop_assets.h"

const CAT_scene test_scene = {
	.origin_x = 0,
	.origin_y = 0,
	.layers = (struct layer[]) {
		{
			.props = (struct prop[]) {
				{
					.prop = &null_prop,
					.position_x = 565,
					.position_y = 470,
				},
			},
			.prop_count = 1,
		},
	},
	.layer_count = 1,
};
