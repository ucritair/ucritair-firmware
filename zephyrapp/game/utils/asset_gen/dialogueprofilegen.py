#!/usr/bin/env python3

import assetgen;

trp = assetgen.Triptych("data/dialogue_profiles.json");

trp.header_intro(["\"cat_dialogue.h\""]);
trp.source_intro(["\"dialogue_assets.h\"", "\"cat_procs.h\""]);

for instance in trp.instances:
	trp.begin_asset_def(instance);

	trp.swrite(f".sprite = &{instance["sprite"]},\n");

	trp.swrite(".blockers = (int16_t*[]) {\n");
	trp.source_indent();
	for blocker in instance["blockers"]:
		trp.swrite("(int16_t[]) {");
		for element in blocker:
			trp.swrite(f"{int(element)},", no_indent=True);
		trp.swrite("},\n", no_indent=True);
	trp.source_unindent();
	trp.swrite("},\n");
	trp.swrite(f".blocker_count = {len(instance["blockers"])},\n");

	trp.swrite(".triggers = (struct trigger[]) {\n");
	trp.source_indent();
	for trigger in instance["triggers"]:
		trp.swrite("{\n");
		trp.source_indent();

		trp.swrite(".aabb = {");
		for element in trigger["aabb"]:
			trp.swrite(f"{element},", no_indent=True);
		trp.swrite("},\n", no_indent=True);

		trp.swrite(f".tx = {trigger["direction"][0]},\n");
		trp.swrite(f".ty = {trigger["direction"][1]},\n");

		trp.swrite(f".proc = {trigger["proc"] if trigger["proc"] != "" else "NULL"},\n");

		trp.source_unindent();
		trp.swrite("},\n");
	trp.source_unindent();
	trp.swrite("},\n");
	trp.swrite(f".trigger_count = {len(instance["triggers"])},\n");

	trp.end_asset_def();