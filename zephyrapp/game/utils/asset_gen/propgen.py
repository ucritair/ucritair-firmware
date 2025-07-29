#!/usr/bin/env python3

import assetgen;

trp = assetgen.Triptych("data/props.json");

trp.header_intro(["\"cat_scene.h\""]);
trp.source_intro(["\"sprite_assets.h\""]);

for instance in trp.instances:
	trp.begin_asset_def(instance);

	trp.swrite(f".sprite = &{instance["sprite"]},\n");

	trp.swrite(".blockers = (int16_t*[]) {\n");
	trp.source_indent();
	for blocker in instance["blockers"]:
		trp.swrite("{");
		for element in blocker:
			trp.swrite(f"{element},");
		trp.swrite("},\n");
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

		trp.swrite(f".proc = {trigger["proc"] if trigger["proc"] != "" else "NULL"},\n");

		trp.source_unindent();
		trp.swrite("},\n");
	trp.source_unindent();
	trp.swrite("},\n");
	trp.swrite(f".trigger_count = {len(instance["triggers"])},\n");

	trp.end_asset_def();