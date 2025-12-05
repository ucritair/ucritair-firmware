#!/usr/bin/env python3

import assetgen;

triptych = assetgen.Triptych(
	"meshes/mesh2ds.json",
	"meshes/mesh2d_assets.h",
	"meshes/mesh2d_assets.c"
);
data = triptych.json_data;
hdr = triptych.header_writer;
src = triptych.source_writer;

hdr.write_header(["\"cat_poly.h\""]);
src.write_header(["\"mesh2d_assets.h\""]);

for instance in data.instances:
	src.start_body(instance["name"]);

	src.start_block("verts", "uint16_t[]");
	for vert in instance["verts"]:
		src.literal(vert);
	src.end_block();
	src.variable("vert_count", instance["vert_count"]);

	src.start_block("edges", "uint16_t[]");
	for edge in instance["edges"]:
		src.literal(edge);
	src.end_block();
	src.variable("edge_count", instance["edge_count"]);

	src.end_body();