#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

json_file = open("data/dialogue.json", "r");
json_data = json.load(json_file);
json_entries = json_data['instances'];
json_file.close();

header = open("data/dialogue_assets.h", "w");
header.write("#pragma once\n");
header.write("\n");
header.write(f"#define DIALOGUE_COUNT {len(json_entries)}\n");
header.write("\n");
header.write("#include \"cat_dialogue.h\"\n");
header.write("#include <stddef.h>\n");
header.write("\n");
for (idx, node) in enumerate(json_entries):
	header.write(f"extern const CAT_dialogue_node dialogue_{node['name']};\n");
header.close();

source = open("data/dialogue_assets.c", "w");
source.write("#include \"dialogue_assets.h\"\n");
source.write("#include \"cat_procs.h\"\n");
source.write("\n");
for (idx, node) in enumerate(json_entries):
	source.write(f"const CAT_dialogue_node dialogue_{node['name']} =\n");
	source.write("{\n");

	source.write(f"\t.lines = (const char*[])\n");
	source.write("\t{\n");
	for line in node["lines"]:
		source.write(f"\t\t\"{line}\",\n");
	source.write("\t},\n");
	source.write(f"\t.line_count = {len(node["lines"])},\n");

	source.write(f"\t.edges = (CAT_dialogue_edge[])\n");
	source.write("\t{\n");
	for edge in node["edges"]:
		source.write("\t\t{\n");
		source.write(f"\t\t\t.text = \"{edge["text"]}\",\n");
		if edge["node"] == "":
			source.write(f"\t\t\t.node = NULL,\n");
		else:
			source.write(f"\t\t\t.node = &dialogue_{edge["node"]},\n");
		if edge["proc"] == "":
			source.write(f"\t\t\t.proc = NULL,\n");
		else:
			source.write(f"\t\t\t.proc = dialogue_proc_{edge["proc"]},\n");
		source.write("\t\t},\n");
	source.write("\t},\n");
	source.write(f"\t.edge_count = {len(node["edges"])},\n");
	source.write("};\n");
	source.write("\n");
source.write("\n");






