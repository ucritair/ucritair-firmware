#!/usr/bin/env python3

import assetgen;

trp = assetgen.Triptych("data/dialogue_profiles.json");

trp.header_intro(["\"cat_dialogue.h\""]);
trp.source_intro(["\"dialogue_assets.h\"", "\"cat_procs.h\""]);

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

for instance in trp.instances:
	trp.begin_asset_def(instance);

	trp.swrite(".entries = (const CAT_dialogue_profile_entry[]) {\n");
	trp.source_indent();
	for entry in instance["entries"]:

		trp.swrite("{\n");
		trp.source_indent();
		trp.swrite(f".node = {transform_dialogue_name(entry["node"])},\n");
		trp.swrite(f".is_active_proc = {transform_proc_name(entry["is_active_proc"])},\n");
		trp.swrite(f".weight = {int(entry["weight"])},\n");
		trp.source_unindent();
		trp.swrite("},\n");
	
	trp.source_unindent();
	trp.swrite("},\n");
	trp.swrite(f".entry_count = {len(instance["entries"])},\n");
	
	trp.swrite();

	trp.swrite(f".mandatory_node = {transform_dialogue_name(instance["mandatory_node"])},\n");
	trp.swrite(f".opener_probability = {instance["opener_probability"]},\n");

	trp.end_asset_def();