#!/usr/bin/env python3

import assetgen;

def transform_dialogue_name(s):
	s = s.replace("##", "");
	if len(s) == 0 or s.isspace():
		return "NULL";
	return f"&dialogue_{s}";

def transform_proc_name(s):
	s.replace("##", "");
	if len(s) == 0 or s.isspace():
		return "NULL";
	return s;

triptych = assetgen.Triptych(
	"data/dialogue_profiles.json",
	"data/dialogue_profile_assets.h",
	"data/dialogue_profile_assets.c"
);
data = triptych.json_data;
hdr = triptych.header_writer;
src = triptych.source_writer;

hdr.write_header(["\"cat_dialogue.h\""]);
src.write_header(["\"dialogue_assets.h\"", "\"cat_procs.h\""]);

for instance in data.instances:
	weight_sum = sum([x["weight"] for x in instance["entries"]]);
	instance["entries"].sort(key = lambda x: x["weight"], reverse=True);

	src.start_body(instance["name"]);
	src.start_block("entries", "struct dialogue_profile_entry[]");
	for entry in instance["entries"]:
		src.start_block();
		src.variable("node", transform_dialogue_name(entry["node"]));
		src.variable("weight", int(weight_sum-entry["weight"]));
		src.end_block();
	src.end_block();
	src.variable("entry_count", len(instance["entries"]));
	src.end_body();