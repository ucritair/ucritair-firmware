#!/usr/bin/env python3

import assetgen;

trp = assetgen.Triptych("data/scenes.json");

trp.header_intro(["\"cat_scene.h\""]);
trp.source_intro(["\"prop_assets.h\""]);

for instance in trp.instances:
	trp.begin_asset_def(instance);

	trp.swrite(".layers = (struct layer[]) {\n");
	trp.source_indent();
	for layer in instance["layers"]:
		trp.swrite("{\n");
		trp.source_indent();

		trp.swrite(".props = (struct prop[]) {\n");
		trp.source_indent();
		for prop in layer:
			trp.swrite("{\n");
			trp.source_indent();

			trp.swrite(f".prop = &{prop["prop"]},\n");
			trp.swrite(f".position_x = {prop["position"][0]},\n");
			trp.swrite(f".position_y = {prop["position"][1]},\n");

			trp.source_unindent();
			trp.swrite("},\n");
		trp.source_unindent();
		trp.swrite("},\n");
		trp.swrite(f".prop_count = {len(layer)},\n");

		trp.source_unindent();
		trp.swrite("},\n");
	trp.source_unindent();
	trp.swrite("},\n");
	trp.swrite(f".layer_count = {len(instance["layers"])},\n");

	trp.end_asset_def();