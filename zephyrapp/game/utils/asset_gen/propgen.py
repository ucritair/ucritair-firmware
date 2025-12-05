#!/usr/bin/env python3

import assetgen;

triptych = assetgen.Triptych(
	"data/props.json",
	"data/prop_assets.h",
	"data/prop_assets.c"
);
data = triptych.json_data;
hdr = triptych.header_writer;
src = triptych.source_writer;

hdr.write_header(["\"cat_scene.h\""]);
src.write_header(["\"sprite_assets.h\"", "\"cat_procs.h\""]);

for instance in data.instances:
	src.start_body(instance["name"]);
	src.variable("sprite", f"&{instance["sprite"]}");

	src.start_block("blockers", "int8_t*[]");
	for blocker in instance["blockers"]:
		src.start_block(type="int8_t[]");
		for element in blocker:
			src.literal(int(element));
		src.end_block();
	src.end_block();
	src.variable("blocker_count", len(instance["blockers"]));

	src.start_block("triggers", "struct trigger[]");
	for trigger in instance["triggers"]:
		src.start_block();
		
		src.start_block("aabb");
		for element in trigger["aabb"]:
			src.literal(int(element));
		src.end_block();

		src.variable("tx", int(trigger["direction"][0]));
		src.variable("ty", int(trigger["direction"][1]));

		src.variable("proc", trigger["proc"] if trigger["proc"] != "" else "NULL");

		src.end_block();
	src.end_block();
	src.variable("trigger_count", len(instance["triggers"]));

	src.end_body();