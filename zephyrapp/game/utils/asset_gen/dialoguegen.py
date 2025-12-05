#!/usr/bin/env python3

import assetgen;

triptych = assetgen.Triptych(
	"data/dialogue.json",
	"data/dialogue_assets.h",
	"data/dialogue_assets.c"
);
data = triptych.json_data;
data.ctype_name = "CAT_dialogue_node";
hdr = triptych.header_writer;
src = triptych.source_writer;

hdr.write_header(["\"cat_dialogue.h\"", "<stddef.h>"]);
src.write_header(["\"dialogue_assets.h\"", "\"cat_procs.h\""]);

for instance in data.instances:
	src.start_body(instance["name"]);

	src.start_block("lines", "const char*[]");
	for line in instance["lines"]:
		src.literal(f"\"{line}\"");
	src.end_block();
	src.variable("line_count", len(instance["lines"]));
	
	src.start_block("edges", "const CAT_dialogue_edge[]");
	for edge in instance["edges"]:
		src.start_block();
		src.variable("text", f"\"{edge["text"]}\"");

		if edge["node"] == "":
			src.variable("node", "NULL");
		else:
			src.variable("node", f"&dialogue_{edge["node"]}");
		
		if edge["proc"] == "":
			src.variable("proc", "NULL");
		else:
			src.variable("proc", edge["proc"]);
		
		src.end_block();
	src.end_block();
	src.variable("edge_count", len(instance["edges"]));

	src.end_body();






