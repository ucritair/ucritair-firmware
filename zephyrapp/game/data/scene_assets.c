#include "scene_assets.h"
#include "prop_assets.h"

const CAT_scene test_scene = {
	.layers = (struct layer[]) {
		{
			.props = (struct prop[]) {
				{
					.prop = &clouds_prop,
					.position_x = -120,
					.position_y = 63,
				},
				{
					.prop = &reed_prop,
					.position_x = -11,
					.position_y = -89,
				},
			},
			.prop_count = 2,
		},
	},
	.layer_count = 1,
};
