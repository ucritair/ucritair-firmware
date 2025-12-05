#!/usr/bin/env python3

import assetgen;

def TOS_RGBA88882RGB565(c):
	if len(c) == 4 and c[3] < 128:
		return 0xdead;
	r = int((c[0] / 255) * 31);
	g = int((c[1] / 255) * 63);
	b = int((c[2] / 255) * 31);
	return (r << 11) | (g << 5) | b;

triptych = assetgen.Triptych(
	"data/scenes.json",
	"data/scene_assets.h",
	"data/scene_assets.c"
);
data = triptych.json_data;
hdr = triptych.header_writer;
src = triptych.source_writer;

hdr.write_header(["\"cat_scene.h\""]);
src.write_header(["\"prop_assets.h\"", "\"cat_render.h\"", "\"sprite_assets.h\""]);

for instance in data.instances:
	src.start_body(instance["name"]);

	x0, y0, x1, y1 = instance["bounds"];
	src.variable("bounds", f"{{{x0}, {y0}, {x1}, {y1}}}");

	src.start_block("background");

	src.variable("colour", TOS_RGBA88882RGB565(instance["background"]["colour"]));
	src.variable("palette", f"&{instance["background"]["palette"]}");
	
	src.start_block("tiles", "struct tile[]");
	for tile in instance["background"]["tiles"]:
		src.start_block();
		src.variable("x", int(tile["position"][0]));
		src.variable("y", int(tile["position"][1]));
		src.variable("frame", int(tile["frame"]));
		src.end_block();
	src.end_block();
	src.variable("tile_count", len(instance["background"]["tiles"]));

	src.end_block();
	src.start_block("layers", "struct layer[]");
	for layer in instance["layers"]:
		src.start_block();
		
		src.start_block("props", "CAT_prop_instance[]");
		for prop in layer:
			src.start_block();
			src.variable("prop", f"&{prop["prop"]}");
			src.variable("position_x", prop["position"][0]);
			src.variable("position_y", prop["position"][1]);
			src.variable("variant", prop["variant"]);
			src.variable("disabled", "false");
			src.end_block();
		src.end_block();
		src.variable("prop_count", len(layer));

		src.end_block();
	src.end_block();
	src.variable("layer_count", len(instance["layers"]));

	src.end_body();