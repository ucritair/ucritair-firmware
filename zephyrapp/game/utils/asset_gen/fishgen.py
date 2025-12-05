#!/usr/bin/env python3

import assetgen;

triptych = assetgen.Triptych(
	"data/fish.json",
	"data/fish_assets.h",
	"data/fish_assets.c"
);
data = triptych.json_data;
hdr = triptych.header_writer;
src = triptych.source_writer;

hdr.write_header(["\"cat_study.h\""]);
hdr.write_list();
src.write_header(["\"fish_assets.h\"", "\"mesh2d_assets.h\""]);

for instance in data.instances:
	src.start_body(instance["name"]);
	src.variable("name", f"\"{instance['display_name']}\"");
	src.variable("proverb", f"\"{instance['proverb']}\"");
	src.variable("grade_constraint", instance['grade_constraint']);
	src.write();
	src.variable("min_length", instance['min_length']);
	src.variable("max_length", instance['max_length']);
	src.variable("min_lustre", instance['min_lustre']);
	src.variable("max_lustre", instance['max_lustre']);
	src.variable("min_wisdom", instance['min_wisdom']);
	src.variable("max_wisdom", instance['max_wisdom']);
	src.write();
	src.variable("mesh", f"&mesh2d_{instance['mesh']}");
	src.end_body();
src.write();

src.write_list();
