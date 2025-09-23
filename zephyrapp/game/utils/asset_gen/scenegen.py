#!/usr/bin/env python3

import assetgen;

trp = assetgen.Triptych("data/scenes.json");
trp.header_intro(["\"cat_scene.h\""]);
trp.source_intro(["\"prop_assets.h\"", "\"cat_render.h\"", "\"sprite_assets.h\""]);

bank = assetgen.Database();
bank.add_document("data/props.json");
bank.add_document("sprites/sprites.json");

for instance in trp.instances:
	trp.begin_asset_def(instance);

	x0, y0, x1, y1 = instance["bounds"];
	trp.swrite(f".bounds = {{{x0}, {y0}, {x1}, {y1}}},\n");

	trp.swrite(".background = {\n");
	trp.source_indent();
	trp.swrite(f".colour = {assetgen.TOS_RGBA88882RGB565(instance["background"]["colour"])},\n");
	trp.swrite(f".palette = &{instance["background"]["palette"]},\n");
	trp.swrite(".tiles = (struct tile[]) {\n");
	trp.source_indent();
	for tile in instance["background"]["tiles"]:
		trp.swrite("{\n");
		trp.source_indent();
		trp.swrite(f".x = {int(tile["position"][0])},\n");
		trp.swrite(f".y = {int(tile["position"][1])},\n");
		trp.swrite(f".frame = {int(tile["frame"])},\n");
		trp.source_unindent();
		trp.swrite("},\n");
	trp.source_unindent();
	trp.swrite("},\n");
	trp.swrite(f".tile_count = {len(instance["background"]["tiles"])},\n");
	trp.source_unindent();
	trp.swrite("},\n");

	trp.swrite(".layers = (struct layer[]) {\n");
	trp.source_indent();
	for layer in instance["layers"]:
		trp.swrite("{\n");
		trp.source_indent();

		trp.swrite(".props = (CAT_prop_instance[]) {\n");
		trp.source_indent();
		for prop in layer:
			trp.swrite("{\n");
			trp.source_indent();

			trp.swrite(f".prop = &{prop["prop"]},\n");
			trp.swrite(f".position_x = {prop["position"][0]},\n");
			trp.swrite(f".position_y = {prop["position"][1]},\n");
			trp.swrite(f".variant = {prop["variant"]},\n");
			trp.swrite(f".disabled = false,\n");

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